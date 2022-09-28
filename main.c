#include <stdio.h>
#include "cimpleXML.h"

int main()
{
    XMLDocument doc;
    XMLAttributesList list;
    XMLAttributesListInit(&list);
    if (XMLDocument_load(&doc, "test.xml"))
    {
        printf("tag: %s, text: %s\n", doc.root->tag, doc.root->innerText);
        for (int i = 0; i < doc.root->attributes.size; ++i)
        {
            XMLAttribute attr = doc.root->attributes.data[i];
            printf("%s\n%s\n", attr.key, attr.value);
        }
    }
    return 0;
}