#ifndef LITTLE_XML_H
#define LITTLE_XML_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//
// Definitions
//
typedef enum _returnErrorCode
{
    SXML_return_FAIL = 0,
    SXML_return_OK = 1
} returnEoorCode;

typedef struct _XMLAttribute
{
    char *key;
    char *value;
} XMLAttribute;

typedef struct _XMLAttributesList
{
    int heapSize;
    int size;
    XMLAttribute *data;
} XMLAttributesList;

typedef struct _XMLNode
{
    char *tag;
    char *innerText;
    struct _XMLNode *parent;
    XMLAttributesList attributes;
} XMLNode;

typedef struct _XMLDocument
{
    XMLNode *root; // pointer to XMLNode struc (XMLNode will be filled by curr_node)
} XMLDocument;

bool XMLDocument_load(XMLDocument *doc, char *path);
void XMLDocument_free(XMLDocument *doc);
XMLNode *XMLNode_new(XMLNode *node);
void XMLNode_free(XMLNode *node);
void XMLAttributesListInit(XMLAttributesList *list);
void XMLAttributesListAdd(XMLAttributesList *list, XMLAttribute *attr);
void XMLAttributeFree(XMLAttribute *attr);

//
// Implementation
//

bool XMLDocument_load(XMLDocument *doc, char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp)
    {
        fprintf(stderr, "Couldn't open the xml file from: %s\n", path);
        return SXML_return_FAIL;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buf = (char *)malloc(sizeof(char) * size + 1);
    fread(buf, 1, size, fp);
    fclose(fp);

    buf[size] = '\0'; // get rid of the stuff after end of xml

    doc->root = XMLNode_new(NULL);

    char lex[256];
    int lexi = 0; // counter index for lex
    int i = 0;    // counter index for buf
    XMLNode *curr_node = NULL;
    while (buf[i] != '\0')
    {
        if (buf[i] == '<')
        {
            // get inner text
            if (lexi > 0)
            {
                lex[lexi] = '\0'; // null-terminate the inner text -> to store it later in 'innerText'
                if (!curr_node)
                {
                    fprintf(stderr, "Text outside of a node!\n");
                    return SXML_return_FAIL;
                }
                curr_node->innerText = strdup(lex);
                lexi = 0;
            }

            // get the end node
            if (buf[i + 1] == '/')
            {
                i += 2;
                while (buf[i] != '>')
                    lex[lexi++] = buf[i++];
                lexi[lex] = '\0';

                // chack if the tag didn't start with </tagName>
                if (!curr_node)
                {
                    fprintf(stderr, "Already at the root, or tag started with an end tag!");
                    return SXML_return_FAIL;
                }
                // check if tag names of the same node is the same or not
                if (strcmp(curr_node->tag, lex))
                {
                    fprintf(stderr, "tag mismatching: open tag: %s and close tag %s", curr_node->tag, lex);
                    return SXML_return_FAIL;
                }

                curr_node = curr_node->parent;
                i++;
                continue;
            }

            // set current node
            if (!curr_node)
                curr_node = doc->root;
            else
                curr_node = XMLNode_new(curr_node);

            // get beginning of tag
            i++; // skip the '<'
            XMLAttribute curr_attr;
            curr_attr.key = NULL;
            curr_attr.value = NULL;
            while (buf[i] != '>')
            {
                lex[lexi++] = buf[i++];

                // tag name
                if (buf[i] == ' ' && !curr_node->tag)
                {
                    lex[lexi] = '\0';
                    curr_node->tag = strdup(lex);
                    lexi = 0;
                    i++;
                    continue;
                }

                // remove possible spaces from out lexi buffer
                if (lex[lexi - 1] == ' ')
                {
                    lexi--;
                    continue;
                }

                if (buf[i] == '=')
                {
                    lex[lexi] = '\0';
                    curr_attr.key = strdup(lex);
                    lexi = 0;
                    continue;
                }

                if (buf[i] == '"')
                {
                    if (!curr_attr.key)
                    {
                        fprintf(stderr, "Value has no key!\n");
                        return SXML_return_FAIL;
                    }
                    lexi = 0;
                    i++;
                    while (buf[i] != '"')
                        lex[lexi++] = buf[i++];
                    lex[lexi] = '\0';
                    curr_attr.value = strdup(lex);
                    XMLAttributesListAdd(&curr_node->attributes, &curr_attr);
                    i++;
                    lexi = 0;
                    continue;
                }
            }
            lex[lexi] = '\0';
            if (!curr_node->tag)
            {
                curr_node->tag = strdup(lex);
            }

            // reset the lexi
            lexi = 0; // finished getting the current tag name, we point to the beginning of lex[],
            // no need to clear the lex[] since we already make \0 after each tag name
            i++; // skip the '>'
            continue;
        }
        else
        {
            lex[lexi++] = buf[i++]; // copy inner text to lex[]
        }
    }

    return SXML_return_OK;
}

void XMLDocument_free(XMLDocument *doc)
{
    free(doc->root);
}

XMLNode *XMLNode_new(XMLNode *parent)
{
    XMLNode *node = (XMLNode *)malloc(sizeof(XMLNode));
    node->parent = parent;
    node->tag = NULL;
    node->innerText = NULL;
    XMLAttributesListInit(&node->attributes);
    return node;
}

void XMLNode_free(XMLNode *node)
{
    if (node->tag)
    {
        free(node->tag);
    }
    if (node->innerText)
    {
        free(node->innerText);
    }
    free(node);
    for (int i = 0; i < node->attributes.size; i++)
        XMLAttributeFree(&node->attributes.data[i]);
}

void XMLAttributesListInit(XMLAttributesList *list)
{
    list->heapSize = 1;
    list->size = 0;
    list->data = (XMLAttribute *)malloc(sizeof(XMLAttribute) * list->heapSize);
}

void XMLAttributesListAdd(XMLAttributesList *list, XMLAttribute *attr)
{
    while (list->size >= list->heapSize)
    {
        list->heapSize *= 2;
        list->data = (XMLAttribute *)realloc(list->data, sizeof(XMLAttribute) * list->heapSize);
    }
    list->data[list->size++] = *attr;
}

void XMLAttributeFree(XMLAttribute *attr)
{
    free(attr->key);
    free(attr->value);
}
#endif