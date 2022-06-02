#include <iostream>

#include "environment.hpp"
#include "namespace.hpp"
#include "symbol.hpp"

namespace xboxdrv {

void init_environment_key(EnvironmentPtr env);

int main()
{
  EnvironmentPtr env(new Environment);

  init_environment_key(env);

  SymbolPtr xbox    = env->lookup_symbol("xbox", "lt");
  SymbolPtr gamepad = env->lookup_symbol("gamepad", "lt");

  std::cout << xbox->str() << std::endl;
  std::cout << gamepad->str() << std::endl;
  std::cout << xbox->match(gamepad) << std::endl;
  std::cout << xbox->match(xbox) << std::endl;
  std::cout << gamepad->match(xbox) << std::endl;
  return 0;
}

} // namespace xboxdrv

/* EOF */
