#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <malloc.h>

#include "parse_xml.h"

static void skip_xml_whitespace(xml_buffer_t* xml_data)
{
    int offset = 0;
    
    for( ; ; )
    {
        offset = xml_data->offset;
        if(isspace(xml_data->buf[offset]))
        {
            xml_data->offset++;
        }
        else
        {
            break;
        }
    }
}

static void skip_xml_space_comments(xml_buffer_t* xml_data)
{
    int orignal_offset;
    int str_len = 0;    


    skip_xml_whitespace(xml_data);

    orignal_offset = xml_data->offset;

    if(xml_data->buf[xml_data->offset] == XML_TAG_BEGIN)    
    {
        if((xml_data->buf[xml_data->offset+1] == XML_TAG_COMMENT_1) || (xml_data->buf[xml_data->offset+1] == XML_TAG_COMMENT_2))
        {
            //skip the commented content
            
            xml_data->offset += 2;
            
            while(xml_data->buf[xml_data->offset] != XML_TAG_END)
            {
               xml_data->offset++;
            }

            //skip the xml end tag ">"
            xml_data->offset++;
        }
    }

    // need to skip whitspace again.
    skip_xml_whitespace(xml_data);
}

static xml_string_t* parse_xml_tag_name(xml_buffer_t* xml_buf)
{
    unsigned int original_offset;
    unsigned int len;
    xml_string_t *tag_name = NULL;

    skip_xml_space_comments(xml_buf);

    original_offset = xml_buf->offset;

    //the tag name string ends with ">" or space symbol
    while(xml_buf->offset < xml_buf->len)
    {
        if((xml_buf->buf[xml_buf->offset] == XML_TAG_END) ||
            (isspace(xml_buf->buf[xml_buf->offset])) )
        {
            break;
        }

        xml_buf->offset++;
    }

    len = xml_buf->offset - original_offset;

    tag_name = (xml_string_t *)malloc(sizeof(xml_string_t));
    tag_name->buf = &xml_buf->buf[original_offset];
    tag_name->len = len;

    //skip the ">" after the name tag
    xml_buf->offset++;

    return tag_name;
}

static xml_string_t* parse_xml_tag_begin(xml_buffer_t* xml_buf)
{
    skip_xml_space_comments(xml_buf);

    //the tag begins with the "<" symbol.
    if(xml_buf->buf[xml_buf->offset] != XML_TAG_BEGIN)
    {
        printf("xml tag doesn't begin with <\n");

        return NULL;
    }

    //skip the "<" tag to parse tag name
    xml_buf->offset++;

    return parse_xml_tag_name(xml_buf);
}

static xml_string_t* parse_xml_tag_end(xml_buffer_t* xml_buf)
{
    skip_xml_space_comments(xml_buf);

    //the tag ends with the "</" symbol.
    if((xml_buf->buf[xml_buf->offset] != XML_TAG_BEGIN) ||
       (xml_buf->buf[xml_buf->offset+1] != XML_TAG_STOP))
    {
        printf("xml tag doesn't end with </\n");

        return NULL;
    }

    //skip the "</" tag to paser tag name
    xml_buf->offset += 2;

    return parse_xml_tag_name(xml_buf);       
}



static xml_string_t* parse_xml_content(xml_buffer_t* xml_buf)
{
    unsigned int original_offset;
    unsigned int len;
    xml_string_t *content = NULL;

    skip_xml_space_comments(xml_buf);

    original_offset = xml_buf->offset;

    //the content ends with the next "<" symbol.
    while(xml_buf->buf[xml_buf->offset] != XML_TAG_BEGIN)
    {
        xml_buf->offset++;
    }

    len = xml_buf->offset - original_offset;

    content = (xml_string_t *)malloc(sizeof(xml_string_t));
    content->buf = &xml_buf->buf[original_offset];
    content->len = len;

    return content;
}

static xml_node_t* create_xml_node(xml_string_t* tag, 
                                          xml_string_t* content,
                                          xml_node_t** subnodes)
{
     xml_node_t *new_node;

     new_node = (xml_node_t *)malloc(sizeof(xml_node_t));

     new_node->tag = tag;
     new_node->content = content;
     new_node->subnodes = subnodes;

     return new_node;
}
                              
static void free_xml_node_creation(xml_string_t* tag_begin,
                                              xml_string_t* tag_end, 
                                              xml_string_t* content,
                                              xml_node_t** subnodes)
{
    xml_node_t **it;

    if(tag_begin != NULL)
    {
        free(tag_begin);
    }

    if(tag_end != NULL)
    {
        free(tag_end);
    }

    if(content != NULL)
    {
        free(content);
    }

    it = subnodes;
    while( *it != NULL)
    {
        free(*it);
        it++;
     }

    free(subnodes);

}

static unsigned char get_xml_character(xml_buffer_t* xml_buf, int offset)
{
    skip_xml_whitespace(xml_buf);
    return xml_buf->buf[xml_buf->offset + offset];
}

static xml_node_t* paser_xml_buf(xml_buffer_t* xml_buf)
{
    xml_string_t* tag_begin = NULL;
    xml_string_t* tag_end = NULL;
    xml_string_t* content = NULL;
    xml_node_t** subnodes;


    subnodes = (xml_node_t **)calloc(1, sizeof(xml_node_t*));
	subnodes[0] = NULL;

    // parse the tag begin 
    tag_begin = parse_xml_tag_begin(xml_buf); 
    if(tag_begin == NULL)
    {
        printf("fail to parse tag open\n");
        
        free_xml_node_creation(tag_begin,tag_end, content, subnodes);

        return NULL;
    }

    // create new node when parsing tag without content such as <tag />
    if((tag_begin->len > 0) && (tag_begin->buf[tag_begin->len-1] == XML_TAG_STOP))
    {
         tag_begin->len--;

         return create_xml_node(tag_begin, content, subnodes);
    }

    // parse the buf as content as it doesn't begin with "<" symbol
    if(get_xml_character(xml_buf, 0) != XML_TAG_BEGIN)
    {
        content = parse_xml_content(xml_buf);

        if(content == NULL)
        {
            printf("fail to parse content\n");
            
            free_xml_node_creation(tag_begin,tag_end, content, subnodes);

            return NULL;
        }
    }
    else
    {
        // parse the subnodes if it doesn't begin with "</".
        while (get_xml_character(xml_buf, 1) != XML_TAG_STOP)
        {
            int subnode_count = 0;
            xml_node_t* subnode;

            subnode = paser_xml_buf(xml_buf);
            if (subnode == NULL)
            {
                printf("fail to pasre sub node\n");

                free_xml_node_creation(tag_begin, tag_end, content, subnodes);

                return NULL;
            }

            //get the count of previous sub noeds 
            while(subnodes[subnode_count] != NULL)
            {
                subnode_count++;
            }

            //increase count for the newly created subnode.
            subnode_count++;
            
            subnodes =(xml_node_t **) realloc(subnodes, sizeof(xml_node_t *)*(subnode_count+1));
            subnodes[subnode_count-1] = subnode;
			subnodes[subnode_count] = NULL;

            //skip the ">" symbol of subnodes
            xml_buf->offset++;
        }
    }

    // parse the tag end
    tag_end = parse_xml_tag_end(xml_buf);
    if(tag_end == NULL)
    {
        printf("fail to parse tag end\n");

        free_xml_node_creation(tag_begin, tag_end, content, subnodes);
        
        return NULL;
    }
    
    // compare the begin tag name and end tag name.
    if((tag_begin->len != tag_end->len) ||
       (strncmp(tag_begin->buf, tag_end->buf, tag_begin->len) != 0))
    {
        printf("inconsistent tag name: %s %s\n", tag_begin->buf, tag_end->buf);
        
        free_xml_node_creation(tag_begin, tag_end, content, subnodes);

        return NULL; 
    }

    free(tag_end);

    return create_xml_node(tag_begin, content, subnodes);
}


xml_buffer_t* get_xml_buffer(char* file_name)
{
    FILE *fp = NULL;
    unsigned int file_len = 0;
    unsigned int buf_offset = 0;
    unsigned char *buf = NULL;
    xml_buffer_t *xml_data;

    fp = fopen(file_name, "rb");
    if(fp == NULL)
    {
       printf("open xml file %s failed\n", file_name);
       return NULL;
    }

    //get length of xml file.
    fseek(fp, 0, SEEK_END);
    file_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    xml_data = (xml_buffer_t*)malloc(sizeof(xml_buffer_t));
    xml_data->len = file_len;
    xml_data->offset = 0;

    buf = malloc(file_len * sizeof(unsigned char));
    if(buf != NULL)
    {
        size_t read = 0;
    
        //fill the content of xml file to the buffer.
        while (!feof(fp))
        {
            read = fread(&buf[buf_offset],sizeof(unsigned char), 1, fp);
            buf_offset += read;
        }
    }

    xml_data->buf = buf;
    
    fclose(fp);

    return xml_data;

}

xml_node_t* get_root_node(xml_buffer_t* xml_buf)
{
     return paser_xml_buf(xml_buf);
}

int get_sub_nodes_count(xml_node_t *node)
{
    int node_count = 0;


    while(node->subnodes[node_count] != NULL)
    {
        node_count++;
    }

    return node_count;
}

xml_node_t* get_sub_node(xml_node_t *node, int index)
{
    int sub_node_count;

    sub_node_count = get_sub_nodes_count(node);

    if(index <= sub_node_count)
    {
        return node->subnodes[index-1];
    }
    else
    {
        return NULL;
    }
}

xml_string_t* get_xml_node_tag(xml_node_t *node)
{
    return node->tag;
}

xml_string_t* get_xml_node_content(xml_node_t *node)
{
    return node->content;
}


void free_xml_node(xml_node_t* node)
{
    xml_node_t **it;

    if(node->tag != NULL)
    {
       free(node->tag);
    }

    if(node->content != NULL)
    {
       free(node->content);
    }

    it = node->subnodes;

    while(*it != NULL)
    {
        free_xml_node(*it);
        it++;
    }

    free(node->subnodes);

    free(node);
}

void free_xml_buf(xml_buffer_t *buf)
{
    if(buf != NULL)
    {
        free(buf->buf);
	    free(buf);
    }
}

int main(int argc, char** argv)
{
    xml_buffer_t* xml_buf;
    xml_node_t* root_node;
    xml_node_t* child_node;
	xml_node_t* grand_child_node;
	int i;
	int subnodes_count = 0;


    xml_buf = get_xml_buffer(argv[1]);
    root_node = get_root_node(xml_buf);
    child_node = get_sub_node(root_node, 1);
	subnodes_count = get_sub_nodes_count(child_node);
	
	printf("child has %d sub nodes\n", get_sub_nodes_count(child_node));

    for (i = 1; i <= subnodes_count; i++)
    {
		grand_child_node = get_sub_node(child_node, i);
	   
	    printf("grandchild[%d] node tag:%.*s, content:%.*s\n", i, grand_child_node->tag->len, grand_child_node->tag->buf, grand_child_node->content->len, grand_child_node->content->buf);
	    printf("grandchild[%d] has %d sub nodes\n", i, get_sub_nodes_count(grand_child_node));
    }

    free_xml_node(root_node);
    free(xml_buf);

    return 0;
}

                                
                                
