#include <stdio.h>
#include "cimpleXML.h"

int main()
{
    XMLDocument doc;
    if (XMLDocument_load(&doc, "test.xml"))
    {
        printf("tag: %s, text: %s", doc.root->tag, doc.root->innerText);
    }
    return 0;
}