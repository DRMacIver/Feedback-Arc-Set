#!/usr/bin/env ruby

require "rubygems"
require "json"
require "smart_colored"
require "trollop"

require 'trollop'

OPTS = Trollop::options do
  opt :"valgrind", "Run tests with valgrind", :default => false
end


BASE = File.dirname(__FILE__)
FAS = File.join(BASE, "fas")

TEST_CASES = ARGV.length > 0 ? ARGV : Dir["#{BASE}/testcases/*.data"]

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
  score, lb, ordering = results.split("\n");

  { :score => score.gsub(/Score: */, "").to_f,
    :ordering => ordering.gsub(/^.+: */, "").gsub(/[^0-9 ]/, "").split.map{|x| x.to_i},
    :runtime => runtime, 
    :valgrind_failed => valgrind_failed
  }
end

TEST_CASES.sort!

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

  valgrind_failed = ft[:valgrind_failed] || ordering_error

  if ft[:score] > best_score
    best_score = ft[:score]
    best_run = ft[:ordering]
    File.open(score_file, "w"){|o| o.puts(JSON.pretty_generate({:score => best_score, :ordering => best_run})) } unless correctness_error
  end
  
  quality_lost = (1 - score / best_score) * 100

  quality_failed = quality_lost > 1
  runtime_failed = ft[:runtime] > 60

  failed ||= valgrind_failed
  failed ||= quality_failed
  failed ||= runtime_failed

  puts File.basename(test).gsub(/.data$/, "")
  puts "  Valgrind:      #{valgrind_failed ? FAILURE : SUCCESS}" if OPTS[:"valgrind"]
  puts "  Loss:     #{"%.2f" % quality_lost} #{quality_failed ? FAILURE : SUCCESS}"
  puts "  Runtime:  #{"%.2f" % ft[:runtime]} #{runtime_failed ? FAILURE : SUCCESS}"
  puts "  Correctness:  #{ordering_error} #{ordering_error ? FAILURE : SUCCESS}"
  puts
end

exit 1 if failed
