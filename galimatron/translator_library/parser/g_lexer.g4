lexer grammar g_lexer;

@postinclude
{
#include <map>
#include <regex>
#include <string>
#include <string_view>
}

@lexer::members
{

// a letter, then letters, digits, and underscores
// the same text as the IDENTIFIER rule. an antlr rule cannot use this constant
// cpp2post_cpp reads it when a tag name must be an identifier
static constexpr const char* identifier_pattern = "[A-Za-z][A-Za-z0-9_]*";

// the quotes a STRING lexeme was written with, and the text inside them
//
struct string_lexeme_t
{
    bool raw = false;
    char quote = '"';
    std::string dashes;
    std::string value;
};

// splits one STRING lexeme. plain is quote, text, quote
// raw is R, quote, dashes, (, text, ), the same dashes, quote
//
static string_lexeme_t split_string_lexeme(const std::string_view lexeme)
{
    static const std::regex plain_string("^(['\"])([\\s\\S]*)\\1$");
    static const std::regex raw_string("^R(['\"])(-*)\\(([\\s\\S]*)\\)\\2\\1$");

    std::match_results<std::string_view::const_iterator> matched;
    if (!lexeme.empty() && lexeme.front() == 'R')
    {
        if (!std::regex_match(lexeme.begin(), lexeme.end(), matched, raw_string))
        {
            return {};
        }
        return {
            .raw = true,
            .quote = *matched[1].first,
            .dashes = matched[2].str(),
            .value = matched[3].str()
        };
    }

    if (!std::regex_match(lexeme.begin(), lexeme.end(), matched, plain_string))
    {
        return {};
    }
    return {
        .quote = *matched[1].first,
        .value = matched[2].str()
    };
}

// keeps the split for this token. the token text becomes the inner value
//
void remember_string_lexeme()
{
    auto lexeme = split_string_lexeme(getText());
    const auto value = lexeme.value;
    string_lexemes_.insert_or_assign(tokenStartCharIndex, std::move(lexeme));
    setText(value);
}

// the split stored for a STRING token. empty when this start index is not a string
//
string_lexeme_t string_lexeme_at(const std::size_t start_index) const
{
    const auto found = string_lexemes_.find(start_index);
    if (found == string_lexemes_.end())
    {
        return {};
    }
    return found->second;
}

std::map<std::size_t, string_lexeme_t> string_lexemes_;

}// @lexer::members

channels
{
    WHITESPACE,
    COMMENTS
}

// keywords

// testing: yes (unit)
//
ROOT          : 'root';
AGGREGATE     : 'aggregate';
VERSIONS      : 'versions';
VERSION       : 'version';

// the version that owns a parked body
//
// testing: yes (unit)
//
BEFORE        : 'before';

// testing: yes (unit)
//
LABEL         : 'label';
STATIC        : 'static';
DYNAMIC       : 'dynamic';
TAG           : 'tag';
TAG_NAME_FROM : 'tag-name-from';

// a tag mode. once is this version, including nested aggregates. local is this aggregate only.
// own is a version word: this aggregate's earlier versions, not the root
// testing: yes (unit)
//
ONCE          : 'once';
LOCAL         : 'local';
OWN           : 'own';

// testing: yes (unit)
//
ORDERING      : 'ordering';

// testing: yes (unit)
//
COMMENT       : 'comment';
HASH          : 'hash';

// testing: yes (unit)
//
DELETED       : 'deleted';
DELETE        : 'delete';

// testing: will (unit)
//
WIZARD        : 'wizard';
CREATE        : 'create';
IN            : 'in';

// copy aggregate ping to pong
//
// testing: yes (unit)
//
COPY          : 'copy';
TO            : 'to';

// continue aggregate ping to pong
//
// testing: yes (unit)
//
CONTINUE      : 'continue';

// delete aggregate physically removes the named chunks
//
// testing: yes (unit)
//
PHYSICALLY    : 'physically';

// create hash force and delete hash force pass a stored hash that does not match
//
// testing: yes (unit)
//
FORCE         : 'force';

// testing: yes (unit)
//
COLLAPSE      : 'collapse';

// symbols / delimeters

// testing: yes (unit)
//
COL     : ':';
SEMI    : ';';
LC      : '{';
RC      : '}';
LP      : '(' ;
RP      : ')' ;
LS      : '[' ;
RS      : ']' ;
EQ      : '=';
DOT     : '.';
COMMA   : ',';
MINUS   : '-';
LESS    : '<';
GREAT   : '>';

// testing: no
//
fragment DECIMAL_DIGIT : [0-9];
fragment DECIMAL : ( [1-9] ) DECIMAL_DIGIT*;
fragment DECIMALS : DECIMAL_DIGIT+;

// string: regular and raw
// testing: yes (unit)
//
STRING
  : ( '\'' .*? '\''
    | '"' .*? '"'
    | 'R' '\'' INNER_RAW_STRING '\''
    | 'R' '"' INNER_RAW_STRING '"'
    ) { remember_string_lexeme(); }
  ;

// a string marked for the template engine
// testing: yes (unit)
//
TEMPLATE_ENGINE : '_te';

fragment INNER_RAW_STRING
  : '(' .*? ')'
  | '-' INNER_RAW_STRING '-'
  ;

// the same text as identifier_pattern above
// testing: will (unit)
//
IDENTIFIER : [A-Za-z][A-Za-z0-9_]*;

// testing: no
//
INTEGER : DECIMALS;

// C++-style comments
// testing: yes (unit)
//
CPP_COMMENT : '//' ~[\r\n]* -> channel(COMMENTS);

// C-style comments
// testing: yes (unit)
//
C_COMMENT : '/*' .*? '*/' -> channel(COMMENTS);

// whitespaces
// testing: yes (unit)
//
WS : [ \t\r\n]+ -> skip;//channel(WHITESPACE);
