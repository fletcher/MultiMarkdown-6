#!/usr/bin/env ruby

require 'optparse'

CURRENT_PATH = File.expand_path(File.dirname(__FILE__))
FALLBACK_PATH = File.join(CURRENT_PATH, "..", "build-xcode", "Debug", "include", "libMultiMarkdown", "libMultiMarkdown.h")

options = {:mode => :nsenum}
OptionParser.new do |parser|
  parser.banner = "Usage: #{$0} [options] path/to/libMultiMarkdown.h"
  
  parser.separator ""
  parser.separator "Without an input path, the script uses this default relative project location:"
  parser.separator "../build-xcode/Debug/include/libMultiMarkdown/libMultiMarkdown.h"
  parser.separator ""
    
  parser.on("-h", "--help", "Prints this help") do
    puts parser
    exit
  end
  
  parser.on("-m", "--mode [MODE]", [:swift, :nsenum],
          "Select generator:",
          "  nsenum    Generates Objective-C NS_ENUM wrappers. (Default)",
          "  swift     Generates Swift enum descriptions.") do |mode|
    options[:mode] = mode
  end
  
  parser.on("-o", "--output [PATH]", String,
            "Write output to file instead of STDOUT. (Optional)") do |path|
    options[:outpath] = path
  end
end.parse!


################################################################################
## Types to perform the conversion
################################################################################


module CustomStringConvertible  
  def extension
    indentation = "            "
    cases = case_descriptions
      .map { |line| indentation + line }
      .join("\n")

    return %Q{
extension #{type_name}: CustomStringConvertible {
    public var description: String {
        switch self {
#{cases}
        }
    }
}}
  end

  private 
  
  def case_descriptions
    cases.map { |c| CustomStringConvertible.description_for(self.type_name, c) }
  end
  
  def indented_case_descriptions(indent)
    case_descriptions
      .map { |line| indent + line }
      .join("\n")
  end
  
  def self.description_for(type_name, enum_case)
    case_name = case_only(enum_case)
    
    # Drop redundant enum base prefixes:
    # ".formatLatex" => ".latex"
    # ".extCritic"   => ".critic"
    swift_case_name = case_name.camelize(drop_prefix: case_name.start_with?("EXT_", "FORMAT_"), 
                                         lowercase_first: true)
    %Q{case .#{swift_case_name}: return "#{type_name}.#{swift_case_name}"}
  end
  
  def self.case_only(line)
    line[/\w+/]
  end
end

class String
  def camelize(drop_prefix: false, lowercase_first: false)
    self
      .split('_')
      .drop(drop_prefix ? 1 : 0)
      .map.with_index { |part, i| 
        if lowercase_first && i == 0
          part.downcase
        else
          part.capitalize
        end }
      .join
  end
end

module NSEnum
  def self.type_name(type)
    type_name = type.camelize
    # Remove plural "S"
    if type_name == "TokenTypes"
      "TokenType"
    elsif type_name == "ParserExtensions"
      "ParserExtension"
    else
      type_name
    end
  end
  
  def type_name
    NSEnum.type_name(type)
  end

  def ns_enum
    base_type_name = NSEnum.type_name(type)
    swift_type_name = base_type_name
    objc_type_name = "MMD6#{base_type_name}"
    
    ns_enum_cases = cases
      .map { |line| NSEnum.case(objc_type_name, line) }
      .join("\n")
      
    return %Q{
typedef NS_ENUM(NSUInteger, #{objc_type_name}) {
#{ns_enum_cases}
} NS_SWIFT_NAME(#{swift_type_name});}
  end

  def self.case(type_name, line)
    if /(?<indent>\s*)(?<casename>\w+)(?<remainder>.*)/ =~ line
      # Drop redundant enum base prefixes:
      # "MMD6OutputFormatFormatLatex"  => "MMD6OutputFormatLatex"
      # "MMD6ParserExtensionExtCritic" => "MMD6ParserExtensionCritic"
      drop_prefix = casename.start_with?("EXT_", "FORMAT_")            
      camelized_case_name = casename.camelize(drop_prefix: drop_prefix)
      return %Q{#{indent}#{type_name}#{camelized_case_name} = #{casename},}
    end
    
    return line
  end
end

class Enum
  include CustomStringConvertible
  include NSEnum
  
  attr_accessor :type, :cases
  
  def initialize(type)
    @type = type
    @cases = []
  end
  
  def <<(line)
    return if !Enum.is_enum_case(line)
    cases << line
  end
  
  def self.is_enum_case(line)
    return false if line.strip.empty?
    return false if line.include?("}")
    return false if line.include?("{")
    return true
  end
end

class Enums
  attr_reader :enums
  
  def initialize(stream)
    lines = stream.readlines
    @enums = parse_enums(lines)
  end
  
  def ns_enums
    @enums.map(&:ns_enum)
  end
  
  def swift_descriptions
    @enums.map(&:extension)
  end
  
  private 

  def parse_enums(lines)
    enums = []
    enumbuffer = nil
    lines.each do |line|
      type = line[/^\s*enum (\w+)\s*\{/, 1]
      if !type.nil?
        enumbuffer = Enum.new(type)
      elsif !enumbuffer.nil?
        if line.start_with?("}")
          enums << enumbuffer
          enumbuffer = nil
        else
          enumbuffer << line
        end
      else
        # nop; discard line
      end
    end
    return enums
  end
end

def file_stream_from_argv
  path = ARGV.shift
  return nil if path.nil?
  File.open(path, "r")
end

def fallback_stream
  return nil unless File.exists?(FALLBACK_PATH)
  File.open(FALLBACK_PATH, "r")
end


################################################################################
## Script execution itself
################################################################################


input  = file_stream_from_argv || fallback_stream
if input.nil?
  puts "Failed to read `#{FALLBACK_PATH}`"
  exit -1
end
enums = Enums.new(input)
input.close


result = if options[:mode] == :nsenum
           enums.ns_enums.join("\n")
         elsif options[:mode] == :swift
           enums.swift_descriptions.join("\n")
         else 
           puts "Illegal mode: #{options[:mode]}"
           exit -1
         end


output = if options[:outpath].nil? 
          $stdout
        else 
          File.open(options[:outpath], "w")
        end
output.puts result
output.close
