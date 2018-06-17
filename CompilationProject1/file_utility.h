/*
|-------------------------------------------------------...----------------------------------------
| File Utility Module Header. This module contain utility functions for the parser file processing.
|--------------------------------------------------------...---------------------------------------
|
*/
#pragma once

#ifndef FILE_UTILITY_H
#define FILE_UTILITY_H

/* initialization */
void init_parser(int fileId);
void initHTMLsemanticFile(FILE *HTMLsemanticFile_Ptr);

/* printing methods */
void print_rule_to_file(char *msg);
void printToSemanticHTML(char *msg);

/* error handlers */
void errorHandler(int (*followMethod)(Token *t), Token *token, char *expectedToken, int isOneTokenExpected);
void errorHandlerSemantic(Token *token, char* msg);




#endif