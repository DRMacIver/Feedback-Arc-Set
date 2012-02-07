#!/usr/bin/env ruby

require "rubygems"
require "json"
require "smart_colored"
require "trollop"

STDOUT.sync = true
STDERR.sync = true

OPTS = Trollop::options do
  opt :"valgrind", "Run tests with valgrind", :default => false
  opt :"summary-only", "Only show summary results", :default => false
end


BASE = File.dirname(__FILE__)
FAS = File.join(BASE, "fas")

TEST_CASES = (ARGV.length > 0 ? ARGV : Dir["#{BASE}/testcases/*.data"]).select{|x| x =~ /\.data$/ }

FAILURE = "FAILURE".colored.red
SUCCESS = "SUCCESS".colored.green

def fas(file)
  t = Time.now
	results = nil

	if OPTS[:"valgrind"]
		results = %x{valgrind --error-exitcode=42 --log-file=#{file.gsub(/.data$/, "")}.valgrind.log #{FAS} < #{file}}
	else
		results = %x{#{FAS} < #{file}}
	end
  valgrind_failed = $?.exitstatus != 0
  runtime = Time.now - t
  score, ordering = results.split("\n");

  { :score => score.gsub(/Score: */, "").to_f,
    :ordering => ordering.gsub(/^.+: */, "").gsub(/[^0-9 ]/, "").split.map{|x| x.to_i},
    :runtime => runtime, 
    :valgrind_failed => valgrind_failed
  }
end

TEST_CASES.sort!

quality_failures = []
runtime_failures = []
correctness_failures = []
losses = []
runtimes = []

failed = false
TEST_CASES.each do |test|
  score_file = test.gsub(/.data$/, ".json")

  best_score = 0
  best_run = []
  write_back = false

  if File.exists?(score_file)
    data = JSON.parse(IO.read(score_file))
    best_score = data["score"]
    best_run = data["ordering"]
  end

  start = Time.now
  ft = fas(test)
  score = ft[:score]

  ordering_error = nil

  values_found = ft[:ordering].sort

  values_found.each_with_index do |x, i|
    if x != i 
      ordering_error = "Found wrong value #{x} at index #{i}"
      break
    end
  end

  valgrind_failed = ft[:valgrind_failed] 

  correctness_failed = valgrind_failed || ordering_error

  if ft[:score] > best_score
    best_score = ft[:score]
    best_run = ft[:ordering]
    File.open(score_file, "w"){|o| o.puts(JSON.pretty_generate({:score => best_score, :ordering => best_run})) } unless correctness_failed
  end
  
  quality_lost = (1 - score / best_score) * 100

  losses << quality_lost
  runtimes << ft[:runtime]

  quality_failed = quality_lost > 3
  runtime_failed = ft[:runtime] > 60

  failed ||= valgrind_failed
  failed ||= quality_failed
  failed ||= runtime_failed

  test_name = File.basename(test).gsub(/.data$/, "")

  correctness_failures << test_name if correctness_failed
  quality_failures << test_name if quality_failed
  runtime_failures << test_name if runtime_failed

  unless OPTS[:"summary-only"]
    puts test_name
    puts "  Valgrind:      #{valgrind_failed ? FAILURE : SUCCESS}" if OPTS[:"valgrind"]
    puts "  Loss:     #{"%.2f" % quality_lost} #{quality_failed ? FAILURE : SUCCESS}"
    puts "  Runtime:  #{"%.2f" % ft[:runtime]} #{runtime_failed ? FAILURE : SUCCESS}"
    puts "  Correctness:  #{correctness_failed} #{correctness_failed ? FAILURE : SUCCESS}"
    puts
  end
end

def report_failures(name, failures)
  return if failures.empty?

  puts "  #{name}: #{failures.join(", ")}"
end

def avg(ls)
  ls.inject(0.0){|x, y| x + y } / ls.length
end

def median(ls)
  ls.sort[ls.length / 2]
end

puts "Runtime:"
puts "  mean   : #{"%.2f" % avg(runtimes)}"
puts "  median : #{"%.2f" % median(runtimes)}"
puts "  max    : #{"%.2f" % runtimes.max}"
puts "Loss:"
puts "  mean   : #{"%.2f" % avg(losses)}"
puts "  median : #{"%.2f" % median(losses)}"
puts "  max    : #{"%.2f" % losses.max}"

if failed
  puts "Failures:" 

  report_failures("Correctness", correctness_failures);
  report_failures("Performance", runtime_failures);
  report_failures("Quality", quality_failures);
  exit 1
end
