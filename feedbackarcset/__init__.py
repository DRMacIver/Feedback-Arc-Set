import ctypes
from ctypes import c_int, c_size_t, c_double
import os.path as p
import numpy as np

lib = ctypes.cdll.LoadLibrary(
    p.abspath(p.join(p.dirname(__file__), "..", "fas.so"))
)

lib.tournament_get.restype = c_double
lib.score_fas_tournament.restype = c_double
lib.condorcet_boundary_from.restype = c_size_t


class Tournament(object):
    def __init__(self, size, debug=False):
        if size <= 0:
            raise ValueError("Expected positive size, got %d" % size)
        if debug:
            lib.enable_fas_tournament_debug(c_int(1))
        self.size = size
        self.tournament = lib.new_tournament(c_int(size))

    def __del__(self):
        lib.del_tournament(self.tournament)

    def __getitem__(self, (i, j)):
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
            self.items.ctypes.data,
            *args
        )

    def pretty_good_optimisation(self):
        if len(self.items) < 15:
            self.table_optimise()
        else:
            self.population_optimise()
        self.local_sort()

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
                    self.ordering.ctypes.data,
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
                self.ordering.ctypes.data,
            )
        return self.__score
