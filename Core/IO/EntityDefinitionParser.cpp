/*
 Copyright (C) 2010-2012 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntityDefinitionParser.h"
#include <assert.h>

namespace TrenchBroom {
    
    int EntityDefinitionToken::asInt() {
        return atoi(data.c_str());
    }
    
    float EntityDefinitionToken::asFloat() {
        return atof(data.c_str());
    }
    
    bool EntityDefinitionTokenizer::nextChar() {
        if (m_state == TS_EOF)
            return false;
        
        if (m_stream.eof()) {
            m_state = TS_EOF;
            return false;
        }
        
        m_stream.get(m_char);
        if (m_char == '\n') {
            m_line++;
            m_column = 0;
        } else {
            m_column++;
        }
        
        return true;
    }
    
    void EntityDefinitionTokenizer::pushChar() {
        if (m_state == TS_EOF)
            m_state = TS_OUTDEF;
        
        m_stream.seekg(-1, ios::cur);
        m_stream.get(m_char);
        if (m_char == '\n') {
            m_line--;
            m_column = 0;
            char c;
            int pos = m_stream.tellg();
            for (int i = 0; i < pos; i++) {
                m_stream.seekg(-i, ios::cur);
                m_stream.get(c);
                if (c == '\n')
                    break;
                m_column++;
            }
            m_stream.seekg(pos, ios::beg);
        } else {
            m_column--;
        }
    }
    
    char EntityDefinitionTokenizer::peekChar() {
        m_stream.seekg(1, ios::cur);
        char c;
        m_stream.get(c);
        m_stream.seekg(-1, ios::cur);
        return c;
    }

    EntityDefinitionToken* EntityDefinitionTokenizer::token(ETokenType type, string* data) {
        m_token.type = type;
        if (data == NULL)
            m_token.data.clear();
        else
            m_token.data = *data;
        m_token.line = m_line;
        m_token.column = m_column;
        m_token.charsRead = m_stream.tellg();
        return &m_token;
    }

    EntityDefinitionTokenizer::EntityDefinitionTokenizer(istream& stream) : m_stream(stream), m_state(TS_OUTDEF), m_line(1), m_column(0) {}
    
    EntityDefinitionToken* EntityDefinitionTokenizer::next() {
        string buffer;
        while (nextChar()) {
            switch (m_state) {
                case TS_OUTDEF:
                    switch (m_char) {
                        case '/':
                            if (peekChar() == '*') {
                                m_state = TS_INDEF;
                                while (m_char != ' ')
                                    nextChar();
                                return token(TT_ED_O, NULL);
                            } else if (peekChar() == '/') {
                                m_state = TS_COM;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case TS_INDEF:
                    switch (m_char) {
                        case '*':
                            if (peekChar() == '/') {
                                nextChar();
                                m_state = TS_OUTDEF;
                                return token(TT_ED_C, NULL);
                            }
                            break;
                        case '(':
                            return token(TT_B_O, NULL);
                        case ')':
                            return token(TT_B_C, NULL);
                        case '{':
                            return token(TT_CB_O, NULL);
                        case '}':
                            return token(TT_CB_C, NULL);
                        case ';':
                            return token(TT_SC, NULL);
                        case '?':
                            return token(TT_QM, NULL);
                        case '\n':
                            return token(TT_NL, NULL);
                        case ',':
                            return token(TT_C, NULL);
                        case ' ':
                        case '\t':
                            break;
                        case '-':
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                            m_state = TS_DEC;
                            buffer.clear();
                            buffer += m_char;
                            break;
                        case '"':
                            m_state = TS_Q_STR;
                            buffer.clear();
                            break;
                        default:
                            m_state = TS_WORD;
                            buffer.clear();
                            buffer += m_char;
                            break;
                    }
                    break;
                case TS_COM:
                    if (m_char == '\n')
                        m_state = TS_OUTDEF;
                    break;
                case TS_WORD:
                    switch (m_char) {
                        case '/':
                            if (peekChar() == '*') {
                                pushChar();
                            } else {
                                buffer += m_char;
                                break;
                            }
                        case '(':
                        case ' ':
                        case '\n':
                        case '\t':
                            m_state = TS_INDEF;
                            pushChar();
                            return token(TT_WORD, &buffer);
                        default:
                            buffer += m_char;
                            break;
                    }
                    break;
                case TS_Q_STR:
                    if (m_char == '"') {
                        m_state = TS_INDEF;
                        return token(TT_STR, &buffer);
                    } else {
                        buffer += m_char;
                    }
                    break;
                case TS_DEC:
                    if (m_char == '.')
                        m_state = TS_FRAC;
                case TS_FRAC: {
                    switch (m_char) {
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                        case '.':
                            buffer += m_char;
                            break;
                        case ')':
                        case '\t':
                        case ',':
                        case ' ': {
                            if (m_state == TS_DEC) {
                                pushChar();
                                m_state = TS_INDEF;
                                return token(TT_DEC, &buffer);
                            } else {
                                pushChar();
                                m_state = TS_INDEF;
                                return token(TT_FRAC, &buffer);
                            }
                            break;
                        }
                        default:
                            m_state = TS_WORD;
                            break;
                    }
                    break;
                }            
                default:
                    break;
            }
        }
        
        return NULL;
    }
    
    EntityDefinitionToken* EntityDefinitionTokenizer::peek() {
        int oldLine = m_line;
        int oldColumn = m_column;
        int oldPos = m_stream.tellg();
        char oldChar = m_char;
        ETokenizerState oldState = m_state;
        
        next();
        
        m_line = oldLine;
        m_column = oldColumn;
        m_stream.seekg(oldPos, ios::beg);
        m_char = oldChar;
        m_state = oldState;
        
        return &m_token;
    }
    
    string EntityDefinitionTokenizer::remainder() {
        assert(m_state == TS_INDEF);
        
        next();
        string buffer;
        while (m_state != TS_EOF && m_char != '*' && peekChar() != '/')
            buffer += next()->data;
        pushChar();
        return buffer;
    }
}
