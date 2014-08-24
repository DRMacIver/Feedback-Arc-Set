import ctypes
from ctypes import c_int, c_size_t, c_double, POINTER
import os.path as p
import numpy as np
import random
import math

lib = ctypes.cdll.LoadLibrary(
    p.abspath(p.join(p.dirname(__file__), "..", "fas.so"))
)


class Tournament(ctypes.Structure):
    pass

lib.new_tournament.restype = POINTER(Tournament)
lib.normalize_tournament.restype = POINTER(Tournament)
lib.tournament_get.restype = c_double
lib.score_fas_tournament.restype = c_double
lib.condorcet_boundary_from.restype = c_size_t
lib.local_sort.restype = c_int
lib.window_optimise.restype = c_int
lib.stride_optimise.restype = c_int
lib.kwik_sort.restype = c_int


class Tournament(object):
    @classmethod
    def load(cls, file):
        if isinstance(file, str):
            file = open(file)

        size = int(file.readline().strip())
        tournament = Tournament(size=size)
        n = 0
        for line in file:
            n += 1
            i, j, x = line.strip().split()
            i = int(i)
            j = int(j)
            x = float(x)
            tournament[i, j] = x
        return tournament

    def normalize(self):
        return Tournament(
            size=self.size,
            tournament=lib.normalize_tournament(self.tournament)
        )

    def __init__(self, size=None, debug=False, tournament=None):
        if size <= 0:
            raise ValueError("Expected positive size, got %d" % size)
        if debug:
            lib.enable_fas_tournament_debug(c_int(1))
        if tournament is None:
            tournament = lib.new_tournament(c_size_t(size))
        self.size = size
        self.tournament = tournament

    def __del__(self):
        try:
            if self.tournament:
                lib.del_tournament(self.tournament)
                self.tournament = None
        except AttributeError:
            pass

    def __getitem__(self, (i, j)):
        assert self.tournament is not None
        i, j = self.__convertindices(i, j)
        return lib.tournament_get(self.tournament, i, j)

    def __setitem__(self, (i, j), x):
        i, j = self.__convertindices(i, j)
        return lib.tournament_set(self.tournament, i, j, c_double(x))

    def __convertindices(self, i, j):
        if i < 0 or j < 0 or i >= self.size or j >= self.size:
            raise ValueError(
                "%d, %d out of bounds [0, %d)" % (i, j, self.size)
            )
        return c_size_t(i), c_size_t(j)

    def optimise(self):
        ordering = np.arange(self.size, dtype=c_size_t)
        with Optimiser(self, ordering) as optimiser:
            optimiser.pretty_good_optimisation()
        return Optimisation(self, ordering)


class Optimiser(object):
    def __init__(self, tournament, items):
        self.tournament = tournament
        self.optimiser = lib.new_optimiser(tournament.tournament)
        self.items = items
        self.__normalized_tournament = None

    @property
    def normalized_tournament(self):
        if self.__normalized_tournament is None:
            self.__normalized_tournament = self.tournament.normalize()
        return self.__normalized_tournament

    def reset(self):
        if self.optimiser:
            lib.reset_optimiser(self.optimiser)

    def close(self):
        if self.optimiser:
            lib.del_optimiser(self.optimiser)
            self.optimiser = None

    def __del__(self):
        self.close()

    def __enter__(self):
        return self

    def __exit__(self, *args, **kwargs):
        self.close()

    def __optimise(self, optimise, *args):
        return optimise(
            self.optimiser,
            c_size_t(len(self.items)),
            self.items.ctypes.data_as(POINTER(c_double)),
            *args
        )

    def condorcet_sample_optimise(self, epsilon, delta):
        """
        Method inspired by
            Crowdsourcing for Participatory Democracies:
            Efficient Elicitation of Social Choice Functions
            David T. Lee, Ashish Goel, Tanja Aitamurto, Helene Landemore

        If this tournament comes from a set of votes and has an epsilon
        condorcet winner this will find a 3-epsilon condorcet winner with
        probability at least 1 - delta.

        There's no compelling argument that this should produce great results
        for a tournament, but it seemed worth a try.
        """
        size = self.tournament.size
        n_rounds = int(math.ceil(
            float(size) / (epsilon ** 2) * math.log(float(size) / delta)
        ))
        round_size = int(math.ceil(
            math.log(n_rounds / delta) / (epsilon ** 2)
        ))
        # Force an odd number so we get clear majorities. This doesn't
        # matter much but is helpful for tie breakers.
        if round_size % 2 == 0:
            round_size += 1

        # Getting more than this many votes in a head to head counts as
        # a win. Note: Everyone can be a winner.
        threshold = 0.5 * (1 - 2 * epsilon) * round_size

        nt = self.normalized_tournament
        scores = np.zeros(shape=self.tournament.size, dtype=int)
        for _ in xrange(n_rounds):
            i = random.randint(0, size-1)
            j = random.randint(0, size-1)
            # We sample round_size members of the population and see which
            # candidate beats each in a head to head
            p = nt[i, j]
            sample = np.random.binomial(round_size, p)
            if sample >= threshold:
                scores[i] += 1
            if (round_size - sample) >= threshold:
                scores[j] += 1
        best_order = np.argsort(scores)
        self.items[:] = best_order[::-1]

    def borda_sample_optimise(self, epsilon, delta):
        """
        Method inspired by
            Crowdsourcing for Participatory Democracies:
            Efficient Elicitation of Social Choice Functions
            David T. Lee, Ashish Goel, Tanja Aitamurto, Helene Landemore

        Essentially we imagine this has come from a population of voters
        and run a simulation which with high probability produces something
        that ranks close to the borda score.

        Produces an epsilon-borda ranking with probability at least 1 - delta
        """
        size = self.tournament.size
        n_rounds = int(math.ceil(
            float(size) / (epsilon ** 2) * math.log(float(size) / delta)
        ))

        nt = self.normalized_tournament
        scores = np.zeros(shape=self.tournament.size, dtype=int)
        for _ in xrange(n_rounds):
            i = random.randint(0, size-1)
            j = random.randint(0, size-1)
            if i == j:
                scores[i] += 1
            else:
                p = nt[i, j]
                if random.random() <= p:
                    scores[i] += 1
                else:
                    scores[j] += 1
        best_order = np.argsort(scores)
        self.items[:] = best_order[::-1]

    def pretty_good_optimisation(self):
        if len(self.items) < 15:
            self.table_optimise()
        else:
            self.condorcet_sample_optimise(0.05, 0.001)

        self.force_connectivity()
        self.stride_optimise(11)
        self.local_sort()
        self.stride_optimise(13)
        self.local_sort()
        self.reset()

        for i in xrange(10):
            changed = 0
            changed |= self.stride_optimise(12)
            changed |= self.stride_optimise(7)
            changed |= self.local_sort()
            self.reset()
            if not changed:
                break
            self.single_move_optimise()

        self.window_optimise(10)
        self.local_sort()

    def force_connectivity(self):
        return self.__optimise(lib.force_connectivity)

    def table_optimise(self):
        return self.__optimise(lib.table_optimise)

    def population_optimise(self, initial_size=50, generations=100):
        return self.__optimise(
            lib.population_optimise,
            initial_size,
            generations
        )

    def local_sort(self):
        return self.__optimise(lib.local_sort)

    def window_optimise(self, window=5):
        return self.__optimise(lib.window_optimise, window)

    def stride_optimise(self, stride=10):
        return self.__optimise(lib.stride_optimise, stride)

    def single_move_optimise(self):
        return self.__optimise(lib.single_move_optimise)

    def kwik_sort(self):
        return self.__optimise(lib.kwik_sort, 0)


class Optimisation(object):
    def __init__(self, tournament, ordering):
        self.tournament = tournament
        self.ordering = ordering
        self.__score = None
        self.__condorcet_sets = None

    def __repr__(self):
        return "Optimisation(%r, score=%f)" % (
            self.condorcet_sets,
            self.score
        )

    @property
    def condorcet_sets(self):
        if not self.__condorcet_sets:
            sets = []
            start = 0

            while start < len(self.ordering):
                end = lib.condorcet_boundary_from(
                    self.tournament.tournament,
                    len(self.ordering),
                    self.ordering.ctypes.data_as(POINTER(c_double)),
                    c_size_t(start)
                ) + 1
                sets.append(tuple(self.ordering[start:end]))
                start = end
            self.__condorcet_sets = tuple(sets)
        return self.__condorcet_sets

    @property
    def score(self):
        if not self.__score:
            self.__score = lib.score_fas_tournament(
                self.tournament.tournament,
                c_size_t(len(self.ordering)),
                self.ordering.ctypes.data_as(POINTER(c_double)),
            )
        return self.__score
