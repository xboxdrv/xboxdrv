#!/usr/bin/ruby -w

require "yaml"

def yaml2code(data, ev_type)
  puts "#include \"environment.hpp\""
  puts "#include \"namespace.hpp\""
  puts "#include \"symbol.hpp\""
  puts ""
  puts "void init_environment_#{ev_type}(EnvironmentPtr env)"
  puts "{"
  # pass1 
  data.each do |namespace|
    puts "  {"
    puts "    NamespacePtr ns = env->add_namespace(\"#{namespace['name']}\");"
    namespace[ev_type].each do |sym|
      puts "    {"
      puts "      SymbolPtr sym = ns->add_symbol(\"#{sym['name']}\");"
      sym['aliases'].each do |al|
        puts "      ns->add_alias(\"#{al}\", sym);";
      end if sym.has_key?('aliases')
      puts "    }"
    end 
    puts "  }"
  end

  # pass2
  data.each do |namespace|
    puts "  {"
    puts "    NamespacePtr ns = env->lookup_namespace(\"#{namespace['name']}\");"
    namespace[ev_type].each do |sym|
      if sym.has_key?('provides')
        puts "    {"
        puts "      SymbolPtr sym = ns->lookup(\"#{sym['name']}\");"
        puts "      assert(sym);"       
        sym['provides'].each do |provides|
          ns, sym = provides.split(".")
          puts "      sym->add_provides(env->lookup_symbol(\"#{ns}\", \"#{sym}\"));"
        end
        puts "    }"
      end
    end if namespace.has_key?(ev_type)
    puts "  }"
  end
  puts "}"
end

data = []
Dir.glob("*.yaml") do |filename|
  begin
    data.concat(YAML::load(File.new(filename)))
    # puts "#{filename}: ok"
  rescue ArgumentError => e
    $stderror.puts "#{filename}: error: #{e}"
  end
end

yaml2code(data, 'key')
puts "\n/* EOF */"

# EOF #
