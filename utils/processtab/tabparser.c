#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "/usr/local/include/songlib/util.h"
#include "tabparser.h"

/******** Variables ********/

int indentLevel = 0;

static int nextAttributeIndex = 0;

/******** Function prototypes ********/

static void addASTCommandToASTTab(ASTCommand *, ASTTab *);    
static void addASTCourseToASTStave(ASTCourse *, ASTStave *);    
static void addASTMeasureToASTCourse(ASTMeasure *, ASTCourse *);

static char peekChar(FILE *);
static char *trimWhiteSpace(char *);
static void skipWhiteSpace(FILE *);

/******** New functions ********/

ASTTab *
newASTTab()
    {    
    ASTTab *t = (ASTTab *) New(sizeof(ASTTab));

    t->commands = 0;
    t->commandCount = 0;

    return t;
    }

ASTCommand *
newASTCommand()
    {    
    ASTCommand *c = (ASTCommand *) New(sizeof(ASTCommand));

    c->commandType = NoCommand;
    c->un.stave = 0;
    c->un.attribute = 0;

    return c;
    }


ASTAttribute *
newASTAttribute()
    {    
    ASTAttribute *a = (ASTAttribute *) New(sizeof(ASTAttribute));

    a->name = 0;
    a->value = 0;

    return a;
    }
    
ASTStave *
newASTStave()
    {    
    ASTStave *s = (ASTStave *) New(sizeof(ASTStave));

    s->courses = 0;
    s->courseCount = 0;
    s->timingCourse = 0;

    return s;
    }
    
ASTCourse *
newASTCourse()
    {    
    ASTCourse *c = (ASTCourse *) New(sizeof(ASTCourse));

    c->measures = 0;
    c->measureCount = 0;

    return c;
    }

ASTMeasure *
newASTMeasure()
    {    
    ASTMeasure *m = (ASTMeasure *) New(sizeof(ASTMeasure));

    m->events = 0;

    return m;
    }

/******* Parsing functions ********/


ASTTab *
parseTab(FILE *in)
    {
    ASTTab *t = newASTTab();
    ASTCommand *c;
    char ch;
    int i = 0;

    while ((ch = peekChar(in)) && ch != EOF)
        {
        i++;
        /* comment or empty line */
        if (ch == '#' || ch == '\n') 
            {
            while ((ch = fgetc(in)) && ch != '\n' && ch != EOF)
                {
//printf("[parseTab] consumed character '%s'\n", ch == EOF ? "EOF" : ch == '\n' ? "\\n" : "unknown char");
                /* consume */
                }
            continue;
            }
//printf("[parseTab] parsing command\n");
        c = parseCommand(in);
        if (c == 0) return 0;
        addASTCommandToASTTab(c, t);
        }
//printf("[parseTab] ended with character '%s'\n", ch == EOF ? "EOF" : ch == '\n' ? "\\n" : "unknown char");
        
    return t;
    }

ASTCommand *
parseCommand(FILE *in)
    {
    ASTCommand *c = newASTCommand();
    char ch;
    ASTAttribute *a = 0;
    ASTStave *s = 0;
    char *comment = 0;
    
    while ((ch = peekChar(in)) && ch != EOF && ch != '\n')
        {
    //printf("[parseCommand] read character: '%c'\n", ch);   
        if (ch == '|')
            {  
            s = parseStave(in);
            if (s == 0) return 0;
            
            c->commandType = StaveCommand;
            c->un.stave = s;
            }
        else if (ch == '#')
            {  
            comment = parseComment(in);
            if (comment == 0) return 0;
            
            c->commandType = CommentCommand;
            c->un.comment = comment;
            }
        else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
            {
            a = parseAttribute(in);  
            if (a == 0) return 0;
            
            c->commandType = AttributeCommand;
            c->un.attribute = a;
            break;
            }   
        else if (isspace(ch))
            {
            skipWhiteSpace(in);
            }
        else
            {
            Fatal("unexpected character '%c' reading beginning of line\n");
            return 0;
            }                 
        }
//printf("[parseCommand] consumed character '%s'\n", ch == EOF ? "EOF" : ch == '\n' ? "\\n" : "unknown char");
    
    return c;
    }

ASTAttribute *
parseAttribute(FILE *in)
    {    
    ASTAttribute *a = newASTAttribute();
    char aNameBuffer[AST_ATTRIBUTE_NAME_LENGTH_MAX];
    int index = 0;
    char ch;
//    printf("[parseAttribute]\n");
    
    /* attribute names must be valid C identifiers */
    
    /* first character must be alpha or _ */
    ch = fgetc(in);
    if (!isalpha(ch) && ch != '_')
        {
        Fatal("unexpected character '%c' reading attribute name; expected alpha or '_' character\n", ch);
        return 0;
        }
    aNameBuffer[index++] = ch;
    
    /* remaining characters must be alpha, numeric, or _ */
    while ((ch = fgetc(in)) && (isalnum(ch) || ch == '_'))
        {
            if (index == AST_ATTRIBUTE_NAME_LENGTH_MAX - 1)
                {
                Fatal("attribute name cannot be longer than %i characters\n", AST_ATTRIBUTE_NAME_LENGTH_MAX - 1);
                return 0;
                }

            aNameBuffer[index++] = ch;
//printf("[parseAttribute] added consumed character to name buffer: '%c'\n", ch);
        }
//printf("[parseAttribute] consumed character '%c'\n", ch);
    
    if (ch == EOF)
        {
        Fatal("unexpected EOF reading attribute name\n");
        return 0;
        }
    if (isspace(ch)) skipWhiteSpace(in);
    
    if (ch != ':')
        {
        Fatal("unexpected character '%c' reading attribute name; expected alphanumeric or '_' character, or ':'\n", ch);
        return 0;
        }
        
    aNameBuffer[index] = '\0';
    a->name = trimWhiteSpace(aNameBuffer);
    
    if (strlen(a->name) == 0)
        {
            Fatal("attribute name must be at least one character long\n");
            return 0;
        }
        
    a->value = parseJSON(in);
    
    if (a->value == 0) return 0;
    
//printf("[parseAttribute] value: '%s'\n", a->value);
    
    a->index = nextAttributeIndex++;
    return a;
    }
    
char *
parseJSON(FILE *in)
    {
    /* This is not a full JSON parser, but instead a simple brace-matcher.
       Its only intended use is to consume input through the end brace of a JSON string.
     */
    
    char ch;
    char buffer[JSON_LENGTH_MAX];
    int index = 0;
    int braceLevel = 0;
    bool inQuotes = false;
    bool jsonStarted = false;
    
    skipWhiteSpace(in);
    
    while ((ch = fgetc(in)))
        {
        if (index == JSON_LENGTH_MAX - 1)
            {
            Fatal("JSON string cannot be longer than %i characters\n", JSON_LENGTH_MAX - 1);
            return 0;
            }
        if (ch == EOF)
            {
            Fatal("unexpected EOF reading JSON string\n");
            return 0;
            }
        if (!jsonStarted && ch != '{')
            {
            Fatal("expected '{', found '%c' in JSON string\n", ch);
            return 0;
            }
        if (ch == '\\') 
            {
            // consume backslash and the next character
            buffer[index++] = ch;
            ch = fgetc(in);
            buffer[index++] = ch;
            if (ch == 'u') // consume 3 extra characters for unicode '\u####'
                {
                buffer[index++] = ch;
                ch = fgetc(in);
                buffer[index++] = ch;
                ch = fgetc(in);
                buffer[index++] = ch;
                ch = fgetc(in);
                }
            }
        else if (ch == '\"')
            {
            inQuotes = !inQuotes;
            buffer[index++] = ch;
            }   
        else if (ch == '{')
            {
            jsonStarted = true;
            if (!inQuotes) braceLevel++;
            buffer[index++] = ch;
            }
        else if (ch == '}')
            {
            if (!inQuotes) braceLevel--;
            buffer[index++] = ch;
            if (braceLevel == 0) break;
            }
        else
            {
            buffer[index++] = ch;
            }
        }
        buffer[index] = '\0';
        return StringDup(buffer);
    }

ASTStave *
parseStave(FILE *in)
    {
    ASTStave *s = newASTStave();
    ASTCourse *c = 0;
    int i, j;
    char ch;
    
    /* parseCourse consumes newline for each line, so if another is found, the stave is finished */
    while ((ch = peekChar(in)) && ch != EOF && ch != '\n')
        {
        c = parseCourse(in);
        if (c == 0) return 0;
                
        /* timing course is the last course in each stave */
        if (strcmp(c->name, "") == 0)
            {
            s->timingCourse = c;
            ch = peekChar(in);
            break;
            }   
        
        /* this is a regular course */
        addASTCourseToASTStave(c, s);              
        }
       
    if (s->timingCourse == 0)
        {
        Fatal("timing course not found in stave\n");
        return 0;
        }  
     
    for (i = 0; i < s->courseCount; i++)
        {
        c = s->courses[i];
        
        /* check measure counts against timing course */
        if (c->measureCount != s->timingCourse->measureCount)
            {
            Fatal("course '%s' has %i measures; timing course has %i measures\n", c->name, c->measureCount, s->timingCourse->measureCount);
            return 0;
            }
            
        /* check measure lengths against timing course */
        for (j = 0; j < c->measureCount; j++)
            {
            if (strlen(c->measures[j]->events) != strlen(s->timingCourse->measures[j]->events))
                {
                Fatal("course '%s' measure has length %i; timing course measure has length %i\n", 
                        c->name, strlen(c->measures[j]->events), strlen(s->timingCourse->measures[j]->events));
                return 0;
                }
            }
        }
    
    /* expect newline or EOF */    
    if (ch != '\n' && ch != EOF)
        {
        /* another course found */  
        if (ch == '|') Fatal("timing course is not the last course in stave\n");
        else Fatal("unexpected '%c'\n", ch);
        return 0;
        }
        
    return s;
    }

ASTCourse *
parseCourse(FILE *in)
    {
    ASTCourse *c = newASTCourse();
    ASTMeasure *m;
    char ch;
    char buffer[AST_COURSE_NAME_LENGTH_MAX];
    int index = 0;
    
    /* expect and consume first pipe character */
    ch = fgetc(in);
    if (ch != '|')
        {
        Fatal("expected '|', found '%c'\n", ch);
        return 0;
        }
    
    while ((ch = fgetc(in)) && ch != '|')
        {
            if (index == AST_COURSE_NAME_LENGTH_MAX - 1)
                {
                Fatal("course name cannot be longer than %i characters\n", AST_COURSE_NAME_LENGTH_MAX - 1);
                return 0;
                }
            if (ch == EOF)
                {
                Fatal("unexpected EOF reading course name\n");
                return 0;
                }
            if (ch == '\n')
                {
                Fatal("unexpected newline reading course name\n");
                return 0;
                }

            buffer[index++] = ch;
        }
        
    buffer[index] = '\0';
    c->name = trimWhiteSpace(buffer);
    
    /* loop over measures */
    while ((ch = peekChar(in)) && ch != EOF && ch != '\n')
        {
        m = parseMeasure(in);
        if (m == 0) return 0;
        
        addASTMeasureToASTCourse(m, c);              
        }
    
    /* expect newline or EOF, and consume newline */
    if (ch == '\n') fgetc(in);
    else if (ch != EOF)
        {
        Fatal("unexpected '%c'\n", ch);
        return 0;
        }
     
    return c;
    }

ASTMeasure *
parseMeasure(FILE *in)
    {
    ASTMeasure *m = newASTMeasure();
    char ch;
    char buffer[AST_MEASURE_LENGTH_MAX];
    int index = 0;
    
    while ((ch = fgetc(in)) && ch != '|')
        {
            if (index == AST_MEASURE_LENGTH_MAX - 1)
                {
                Fatal("measure cannot be longer than %i characters\n", AST_MEASURE_LENGTH_MAX - 1);
                return 0;
                }
            if (ch == EOF)
                {
                Fatal("unexpected EOF reading measure (ensure last character on the line is '|')\n");
                return 0;
                }
            if (ch == '\n')
                {
                Fatal("unexpected newline reading measure (ensure last character on the line is '|')\n");
                return 0;
                }

            buffer[index++] = ch;
        }
        
    buffer[index] = '\0';
    m->events = StringDup(buffer);
    
    return m;
    }
    
char *
parseComment(FILE *in)
    {
    char ch;
    char buffer[COMMENT_LENGTH_MAX];
    int index = 0;
    
    while ((ch = peekChar(in)) && ch != '\n' && ch != EOF)
        {
        if (index == COMMENT_LENGTH_MAX - 1)
            {
            Fatal("Comment string cannot be longer than %i characters\n", COMMENT_LENGTH_MAX - 1);
            return 0;
            }
        ch = fgetc(in);
        buffer[index++] = ch;
        }
    return StringDup(buffer);
    }

/******** Growth functions ********/

static void
addASTCommandToASTTab(ASTCommand *c, ASTTab *t)
    {    
    int i;
    ASTCommand **oldCommands = t->commands;
    t->commands = (ASTCommand **) New(sizeof(ASTCommand *) * (t->commandCount+1));
    for (i=0;i<t->commandCount;++i)
        {
        t->commands[i] = oldCommands[i];
        }
    t->commands[t->commandCount] = c;
    ++(t->commandCount);
    }
    
static void
addASTCourseToASTStave(ASTCourse *c, ASTStave *s)
    {    
    int i;
    ASTCourse **oldCourses = s->courses;
    s->courses = (ASTCourse **) New(sizeof(ASTCourse *) * (s->courseCount+1));
    for (i=0;i<s->courseCount;++i)
        {
        s->courses[i] = oldCourses[i];
        }
    s->courses[s->courseCount] = c;
    ++(s->courseCount);
    }
    
static void
addASTMeasureToASTCourse(ASTMeasure *m, ASTCourse *c)
    {    
    int i;
    ASTMeasure **oldMeasures = c->measures;
    c->measures = (ASTMeasure **) New(sizeof(ASTMeasure *) * (c->measureCount+1));
    for (i=0;i<c->measureCount;++i)
        {
        c->measures[i] = oldMeasures[i];
        }
    c->measures[c->measureCount] = m;
    ++(c->measureCount);
    }
    
/******** File reading ********/

static char
peekChar(FILE *in)
    {
    char ch;    
    ch = fgetc(in);
    ungetc(ch, in);
    return ch;
    }
 
static char *
trimWhiteSpace(char *str)
    {
    int frontIndex = 0;
    int backIndex = 0;
    char ch;
    char *newString;
    int i;
    
    if (str == 0) return 0;
    
    while ((ch = str[backIndex]) && ch != '\0' && isspace(ch))
        {
        frontIndex++;
        backIndex++;
        }
        
    if (ch == '\0') return "";
        
    backIndex++;
        
    while ((ch = str[backIndex]) && ch != '\0')
        {
        backIndex++;
        }

    while ((ch = str[backIndex-1]) && isspace(ch) && backIndex > frontIndex)
        {
        backIndex--;
        }
    newString  = (char *)New(sizeof(char) * (backIndex-frontIndex));
    
    for (i = 0; i < backIndex - frontIndex; i++)
        {
        newString[i] = str[frontIndex + i];
        }
    
    newString[backIndex - frontIndex] = '\0';
    return newString;
    }
    
static void
skipWhiteSpace(FILE *in)
    {
    int ch;

    while ((ch = fgetc(in)) && ch != EOF && isspace(ch))
        {
        }

    ungetc(ch,in);
    }
    
/******** TESTING ********/

void 
printASTTab(ASTTab *tab, FILE *out)
	{
	int i, j, k;
	
    if (tab == 0) 
    	{
    	Fatal("'tab' parameter is null\n");
    	return;
    	}
    	
    indent(out); fprintf(out, "ASTTab\n");
    indentLevel++;
    indent(out); fprintf(out, "commandCount: %i\n", tab->commandCount);        
    for (i = 0; i < tab->commandCount; i++)
        {
        indent(out); fprintf(out, "ASTCommand\n");
        indentLevel++;
        indent(out); fprintf(out, "type: %i\n", tab->commands[i]->commandType); 
        if (tab->commands[i]->commandType == AttributeCommand)
            {
            indent(out); fprintf(out, "ASTAttribute (not supported)\n");
            indentLevel++;
            indent(out); fprintf(out, "name: %s\n", tab->commands[i]->un.attribute->name);  
            indent(out); fprintf(out, "value: %s\n", tab->commands[i]->un.attribute->value);  
            indentLevel--;
            }
        else if (tab->commands[i]->commandType == StaveCommand)
            {
            indent(out); fprintf(out, "ASTStave\n");
            indentLevel++;
            indent(out); fprintf(out, "courseCount: %i\n", tab->commands[i]->un.stave->courseCount);  
            for(j=0;j<tab->commands[i]->un.stave->courseCount;j++)
                {
                indent(out); fprintf(out, "ASTCourse\n");
                indentLevel++;
                indent(out); fprintf(out, "name: %s\n", tab->commands[i]->un.stave->courses[j]->name); 
                indent(out); fprintf(out, "measureCount: %i\n", tab->commands[i]->un.stave->courses[j]->measureCount);                
                for(k=0;k<tab->commands[i]->un.stave->courses[j]->measureCount;k++)
                    {
                    indent(out); fprintf(out, "ASTMeasure\n");
                    indentLevel++;
                    indent(out); fprintf(out, "events: %s\n", tab->commands[i]->un.stave->courses[j]->measures[k]->events);
                    indentLevel--;
                    }
                indentLevel--;
                }
              
            indent(out); fprintf(out, "timingCourse:\n");
            indentLevel++;
            indent(out); fprintf(out, "ASTCourse\n");
            indentLevel++;
            indent(out); fprintf(out, "measureCount: %i\n", tab->commands[i]->un.stave->timingCourse->measureCount);                
            for(k=0;k<tab->commands[i]->un.stave->timingCourse->measureCount;k++)
                {
                indent(out); fprintf(out, "ASTMeasure\n");
                indentLevel++;
                indent(out); fprintf(out, "events: %s\n", tab->commands[i]->un.stave->timingCourse->measures[k]->events);
                indentLevel--;
                }
            indentLevel--;
            indentLevel--;
            indentLevel--;
            }
        else if (tab->commands[i]->commandType == CommentCommand)
            {
            indent(out); fprintf(out, "Comment:\n");
            indentLevel++;
            indent(out); fprintf(out, "%s\n", tab->commands[i]->un.comment);    
            indentLevel--;
            }
        else
            {
            Fatal("*** command type %i not recognized! ***\n", tab->commands[i]->commandType);
            return;
            }

        indentLevel--;
        }
    indentLevel--;
    }
    
void
indent(FILE *out)
	{
    int i;
    if (indentLevel < 0)
    	{
    	Fatal("indentLevel less than zero, actual value: %i\n", indentLevel);
    	return;
    	}
    for (i = 0; i < indentLevel; i++)
        {
        fprintf(out, "    ");        
        }
    }
