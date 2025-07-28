/* Copyright (c) 2024 No Tomorrow Games. All rights reserved. */

#pragma once

#include <core/json/parser/SourceStream.hpp>
#include <core/json/parser/TokenStream.hpp>
#include <core/json/parser/SourceLocation.hpp>
#include <core/json/parser/CompilationUnit.hpp>

#include <util/UTF8.hpp>

namespace hyperion::json {

class Lexer
{
public:
    Lexer(
        const SourceStream& sourceStream,
        TokenStream* tokenStream,
        CompilationUnit* compilationUnit);
    Lexer(const Lexer& other);

    /** Forms the given TokenStream from the given SourceStream */
    void Analyze();
    /** Reads the next token and returns it */
    Token NextToken();
    /** Reads two characters that make up an escape code and returns actual value */
    utf::u32char ReadEscapeCode();
    /** Reads a string literal and returns the token */
    Token ReadStringLiteral();
    /** Reads a number literal and returns the token */
    Token ReadNumberLiteral();
    /** Reads a hex number literal and returns the token */
    Token ReadHexNumberLiteral();
    /** Reads a single-line comment */
    Token ReadLineComment();
    /** Reads a multi-line block comment */
    Token ReadBlockComment();
    /** Reads an important comment (documentation block) */
    Token ReadDocumentation();
    /** Reads the name, and returns the either identifier or keyword token */
    Token ReadIdentifier();

private:
    SourceStream m_sourceStream;
    TokenStream* m_tokenStream;
    CompilationUnit* m_compilationUnit;
    SourceLocation m_sourceLocation;

    /** Adds an end-of-file error if at the end, returns true if not */
    bool HasNext();
    /** Reads until there is no more whitespace.
        Returns true if a newline character was encountered.
    */
    bool SkipWhitespace();
};

} // namespace hyperion::json
