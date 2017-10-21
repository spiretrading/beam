#ifndef BEAM_LUA_REACTOR_PARAMETER_HPP
#define BEAM_LUA_REACTOR_PARAMETER_HPP
extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}
#include <memory>
#include <boost/noncopyable.hpp>
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  /*! \class LuaReactorParameter
      \brief Base class for a parameter passed into a Lua script.
   */
  class LuaReactorParameter : private boost::noncopyable {
    public:
      virtual ~LuaReactorParameter() = default;

      //! Returns the Reactor representing the Lua parameter.
      const std::shared_ptr<BaseReactor>& GetReactor() const;

      //! Pushes the parameter onto the Lua stack.
      virtual void Push(lua_State& luaState) const = 0;

    protected:

      //! Constructs a LuaReactorParameter.
      /*!
        \param reactor The Reactor representing the parameter.
      */
      LuaReactorParameter(std::shared_ptr<BaseReactor> reactor);

    private:
      std::shared_ptr<BaseReactor> m_reactor;
  };

  inline const std::shared_ptr<BaseReactor>&
      LuaReactorParameter::GetReactor() const {
    return m_reactor;
  }

  inline LuaReactorParameter::LuaReactorParameter(
      std::shared_ptr<BaseReactor> reactor)
      : m_reactor{std::move(reactor)} {}
}
}

#endif
