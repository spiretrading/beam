#ifndef BEAM_PARSER_BOX_HPP
#define BEAM_PARSER_BOX_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/ParserStreamBox.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam::Parsers {

  /**
   * Provides a generic interface over an arbitrary Parser.
   * @param <R> The type to parse.
   */
  template<typename R>
  class ParserBox {
    public:
      using Result = R;

      /**
       * Constructs a ParserBox of a specified type using emplacement.
       * @param <T> The type of parser to emplace.
       * @param args The arguments to pass to the emplaced parser.
       */
      template<typename T, typename... Args>
      explicit ParserBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ParserBox by copying an existing parser.
       * @param parser The parser to copy.
       */
      template<typename Parser>
      explicit ParserBox(Parser parser);

      explicit ParserBox(ParserBox* parser);

      explicit ParserBox(const std::shared_ptr<ParserBox>& parser);

      explicit ParserBox(const std::unique_ptr<ParserBox>& parser);

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;

      template<typename T, typename = disable_copy_constructor_t<ParserBox, T>>
      ParserBox& operator =(T&& parser);

    private:
      struct VirtualParser {
        virtual ~VirtualParser() = default;
        virtual bool Read(ParserStreamBox& source, Result& value) const = 0;
        virtual bool Read(ParserStreamBox& source) const = 0;
      };
      template<typename P>
      struct WrappedParser final : VirtualParser {
        GetOptionalLocalPtr<P> m_parser;

        template<typename... Args>
        WrappedParser(Args&&... args);
        bool Read(ParserStreamBox& source, Result& value) const override;
        bool Read(ParserStreamBox& source) const override;
      };
      std::shared_ptr<VirtualParser> m_parser;
  };

  template<typename R>
  template<typename T, typename... Args>
  ParserBox<R>::ParserBox(std::in_place_type_t<T>, Args&&... args)
    : m_parser(std::make_shared<WrappedParser<T>>(
        std::forward<Args>(args)...)) {}

  template<typename R>
  template<typename Parser>
  ParserBox<R>::ParserBox(Parser parser)
    : ParserBox(std::in_place_type<Parser>, std::move(parser)) {}

  template<typename R>
  ParserBox<R>::ParserBox(ParserBox* parser)
    : ParserBox(*parser) {}

  template<typename R>
  ParserBox<R>::ParserBox(const std::shared_ptr<ParserBox>& parser)
    : ParserBox(*parser) {}

  template<typename R>
  ParserBox<R>::ParserBox(const std::unique_ptr<ParserBox>& parser)
    : ParserBox(*parser) {}

  template<typename R>
  template<typename Stream>
  bool ParserBox<R>::Read(Stream& source, Result& value) const {
    auto sourceBox = ParserStreamBox(&source);
    return m_parser->Read(sourceBox, value);
  }

  template<typename R>
  template<typename Stream>
  bool ParserBox<R>::Read(Stream& source) const {
    auto sourceBox = ParserStreamBox(&source);
    return m_parser->Read(sourceBox);
  }

  template<typename R>
  template<typename T, typename>
  ParserBox<R>& ParserBox<R>::operator =(T&& parser) {
    auto box = ParserBox<Result>(std::forward<T>(parser));
    m_parser.swap(box.m_parser);
    return *this;
  }

  template<typename R>
  template<typename P>
  template<typename... Args>
  ParserBox<R>::WrappedParser<P>::WrappedParser(Args&&... args)
    : m_parser(std::forward<Args>(args)...) {}

  template<typename R>
  template<typename P>
  bool ParserBox<R>::WrappedParser<P>::Read(ParserStreamBox& source,
      Result& value) const {
    return m_parser.Read(source, value);
  }

  template<typename R>
  template<typename P>
  bool ParserBox<R>::WrappedParser<P>::Read(ParserStreamBox& source) const {
    return m_parser.Read(source);
  }
}

#endif
