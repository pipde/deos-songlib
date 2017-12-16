#ifndef INCLUDED_TABPARSER_H
#define INCLUDED_TABPARSER_H

#include <stdio.h>

/******** Constants ********/

#define AST_ATTRIBUTE_NAME_LENGTH_MAX 256
#define AST_COURSE_NAME_LENGTH_MAX 256
#define AST_MEASURE_LENGTH_MAX 1024
#define JSON_LENGTH_MAX 65536
#define COMMENT_LENGTH_MAX 4096

/******** Variables ********/

extern int indentLevel;

/******** Struct prototypes ********/

typedef struct ast_tab ASTTab;
typedef struct ast_command ASTCommand;
typedef struct ast_attribute ASTAttribute;
typedef struct ast_stave ASTStave;
typedef struct ast_course ASTCourse;
typedef struct ast_measure ASTMeasure;

/******** Enum definitions ********/
    
typedef enum command_type
    {
    NoCommand = 0,
    AttributeCommand = 1,
    StaveCommand = 2,
    CommentCommand = 3
    } CommandType;

/******** AST Structs ********/
    
struct ast_tab
    {
    ASTCommand **commands;
    int commandCount;
    };
 
struct ast_command
    {
    CommandType commandType;
    union
        {
        ASTAttribute *attribute;
        ASTStave *stave;
        char *comment;
        } un;
    };
   
struct ast_attribute
    {
    char *name;
    char *value;
    int index;
    };
    
struct ast_stave
    {
    ASTCourse **courses;
    int courseCount;
    ASTCourse *timingCourse;
    };
    
struct ast_course
    {
    char *name;
    ASTMeasure **measures;
    int measureCount;
    };
    
struct ast_measure
    {
    char *events;
    };
   
/******** Tab Parsing API ********/

extern ASTTab *newASTTab();
extern ASTCommand *newASTCommand();
extern ASTAttribute *newASTAttribute();
extern ASTStave *newASTStave();
extern ASTCourse *newASTCourse();
extern ASTMeasure *newASTMeasure();

extern ASTTab *parseTab(FILE *);
extern ASTCommand *parseCommand(FILE *);
extern ASTAttribute *parseAttribute(FILE *);
extern char *parseJSON(FILE *);
extern ASTStave *parseStave(FILE *);
extern ASTCourse *parseCourse(FILE *);
extern ASTMeasure *parseMeasure(FILE *);
extern char *parseComment(FILE *);

extern void printASTTab(ASTTab *, FILE *);
extern void indent(FILE *);

#endif	/* INCLUDED_TABPARSER_H */
