#pragma once

#include <cstddef>
#include <stdexcept>
#include <iostream>

namespace sts
{
  namespace cmd
  {
    struct summary
    {
      std::string name;
      std::size_t limit{};
    };

    struct help_request
    { std::string name; };
    struct version_request
    { std::string name; };

    summary parse(int argc, char **argv)
    {
      if(argc < 1)
      { throw std::underflow_error{ "invalid number of arguments" }; }
      
      summary sum;
      sum.name = argv[0];
      --argc; ++argv;

      try
      {
        for(int i{}; i < argc; ++i)
        {
          std::string const arg{ argv[i] };
          if(arg == "-h" || arg == "--help")
          { throw help_request{ sum.name }; }
          else if(arg == "-u" || arg == "--unlimited")
          { sum.limit = 0; }
          else if(arg == "-l" || arg == "--limit")
          {
            if(i + 1 == argc)
            { throw std::invalid_argument{ "invalid limit specifier; use " + arg + " buf_limit" }; }
            auto const limit(std::stoll(argv[i + 1]));
            if(limit < 0)
            { throw std::invalid_argument{ "invalid limit; must not be negative" }; }
            sum.limit = limit;
            ++i;
          }
          else if(arg == "-v" || arg == "--version")
          { throw version_request{ sum.name }; }
          else
          { throw help_request{ sum.name }; }
        }
      }
      catch(std::invalid_argument const &e)
      {
        std::cout << "error: " << e.what() << std::endl << std::endl;
        throw help_request{ sum.name };
      }

      return sum;
    }

    [[noreturn]] void show_help(help_request const &hr)
    {
      std::cout << hr.name << " v" << VERSION << std::endl
      << "usage:\n  " << hr.name << " [-u] [-l buf_limit]\n"
      << std::endl
      << "options:\n"
      << "  -h,  --help                         Show this help message and exit\n"
      << "  -u,  --unlimited                    [default] Enable unlimited backlog\n"
      << "  -l buf_limit, --limit buf_limit     [default = 0] Specify backlog limit in lines\n" ;
      std::exit(0);
    }

    [[noreturn]] void show_version(version_request const &vr)
    {
      std::cout << vr.name << " v" << VERSION
                << " compiled " << __DATE__
                << std::endl;
      std::exit(0);
    }
  }
}
