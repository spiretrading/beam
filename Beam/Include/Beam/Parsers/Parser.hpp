#ifndef BEAM_PARSER_HPP
#define BEAM_PARSER_HPP
#include <concepts>
#include <memory>
#include <utility>
#include "Beam/Parsers/ParserStream.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** Concept satisfied by types implementing a Parser. */
  template<typename P>
  concept IsParser = requires {
    typename P::Result;
  } && requires(const P& p) {
    { p.read(std::declval<ParserStream&>()) } -> std::convertible_to<bool>;
  } && (std::same_as<typename P::Result, void> || requires(const P& p) {
    { p.read(
        std::declval<ParserStream&>(), std::declval<typename P::Result&>()) } ->
          std::convertible_to<bool>;
  });

  /**
   * Concept satisfied by parser types for a given result type.
   * @tparam R The data type storing the parsed value.
   */
  template<typename P, typename R>
  concept IsParserOf = IsParser<P> && std::same_as<typename P::Result, R>;

  /**
   * Parses values from a stream.
   * @tparam R The data type storing the parsed value.
   */
  template<typename R>
  class Parser {
    public:

      /** The data type storing the parsed value. */
      using Result = R;

      /**
       * Constructs a Parser of a specified type using emplacement.
       * @tparam P The type of parser to emplace.
       * @param args The arguments to pass to the emplaced parser.
       */
      template<IsParserOf<R> P, typename... Args>
      explicit Parser(std::in_place_type_t<P>, Args&&... args);

      /**
       * Constructs a Parser by referencing an existing parser.
       * @tparam P The type of parser to reference.
       * @param parser The parser to reference.
       */
      template<DisableCopy<Parser<R>> P>
        requires IsParserOf<dereference_t<P>, R>
      Parser(P&& parser);

      Parser(const Parser&) = default;
      Parser(Parser&&) = default;

      /**
       * Parses the next value from a stream.
       * @param source The stream to parse from.
       * @param result Stores the parsed value.
       * @return <code>true</code> if a value was parsed.
       */
      template<IsParserStream S>
      bool read(S& source, Result& result) const;

      /**
       * Parses the next value from a stream.
       * @param source The stream to parse from.
       * @return <code>true</code> if a value was parsed.
       */
      template<IsParserStream S>
      bool read(S& source) const;

    private:
      struct VirtualParser {
        virtual ~VirtualParser() = default;

        virtual bool read(ParserStream&, Result&) const = 0;
        virtual bool read(ParserStream&) const = 0;
      };
      template<typename P>
      struct WrappedParser final : VirtualParser {
        using ParserType = P;
        local_ptr_t<P> m_parser;

        template<typename... Args>
        WrappedParser(Args&&... args);

        bool read(ParserStream& stream, Result& result) const override;
        bool read(ParserStream& stream) const override;
      };
      VirtualPtr<VirtualParser> m_parser;
  };

  template<>
  class Parser<void> {
    public:
      using Result = void;

      template<IsParserOf<void> P, typename... Args>
      explicit Parser(std::in_place_type_t<P>, Args&&... args)
        : m_parser(
            make_virtual_ptr<WrappedParser<P>>(std::forward<Args>(args)...)) {}

      template<DisableCopy<Parser> P> requires
        IsParserOf<dereference_t<P>, void>
      Parser(P&& parser)
        : m_parser(make_virtual_ptr<WrappedParser<std::remove_cvref_t<P>>>(
            std::forward<P>(parser))) {}

      Parser(const Parser&) = default;

      template<IsParserStream S>
      bool read(S& source) const {
        auto parser_stream = ParserStream(source);
        return m_parser->read(parser_stream);
      }

    private:
      struct VirtualParser {
        virtual ~VirtualParser() = default;
        virtual bool read(ParserStream&) const = 0;
      };
      template<typename P>
      struct WrappedParser final : VirtualParser {
        using ParserType = P;
        local_ptr_t<P> m_parser;

        template<typename... Args>
        WrappedParser(Args&&... args)
          : m_parser(std::forward<Args>(args)...) {}

        bool read(ParserStream& stream) const override {
          return m_parser->read(stream);
        }
      };
      VirtualPtr<VirtualParser> m_parser;
  };

  template<typename R>
  template<IsParserOf<R> P, typename... Args>
  Parser<R>::Parser(std::in_place_type_t<P>, Args&&... args)
    : m_parser(
        make_virtual_ptr<WrappedParser<P>>(std::forward<Args>(args)...)) {}

  template<typename R>
  template<DisableCopy<Parser<R>> P> requires IsParserOf<dereference_t<P>, R>
  Parser<R>::Parser(P&& parser)
    : m_parser(make_virtual_ptr<WrappedParser<std::remove_cvref_t<P>>>(
        std::forward<P>(parser))) {}

  template<typename R>
  template<IsParserStream S>
  bool Parser<R>::read(S& source, Result& result) const {
    auto parser_stream = ParserStream(&source);
    return m_parser->read(parser_stream, result);
  }

  template<typename R>
  template<IsParserStream S>
  bool Parser<R>::read(S& source) const {
    auto parser_stream = ParserStream(&source);
    return m_parser->read(parser_stream);
  }

  template<typename R>
  template<typename P>
  template<typename... Args>
  Parser<R>::WrappedParser<P>::WrappedParser(Args&&... args)
    : m_parser(std::forward<Args>(args)...) {}

  template<typename R>
  template<typename P>
  bool Parser<R>::WrappedParser<P>::read(
      ParserStream& stream, Result& result) const {
    return m_parser->read(stream, result);
  }

  template<typename R>
  template<typename P>
  bool Parser<R>::WrappedParser<P>::read(ParserStream& stream) const {
    return m_parser->read(stream);
  }
}

#endif
