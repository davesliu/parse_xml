#ifndef __PARSE_XML_H__
#define __PARSE_XML_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>


#define XML_TAG_BEGIN      '<'
#define XML_TAG_END        '>'
#define XML_TAG_STOP       '/'
#define XML_TAG_COMMENT_1  '!'
#define XML_TAG_COMMENT_2  '?'
#define XML_TAG_ID         ':'
	
struct xml_string
{
   unsigned char* buf;
   unsigned int   len;
};

struct xml_buffer
{
	unsigned char* buf;
	unsigned int   offset;
	unsigned int   len;
};

typedef struct xml_string xml_string_t;

struct xml_node
{
   xml_string_t*	  tag;
   xml_string_t*	  content;
   struct xml_node**  subnodes;
};

typedef struct xml_string xml_string_t;
typedef struct xml_buffer xml_buffer_t;
typedef struct xml_node   xml_node_t;

xml_buffer_t* get_xml_buffer(char* file_name);
xml_node_t* get_root_node(xml_buffer_t* xml_buf);
int get_sub_nodes_count(xml_node_t *node);
xml_node_t* get_sub_node(xml_node_t *node, int index);
xml_string_t* get_xml_node_tag(xml_node_t *node);
xml_string_t* get_xml_node_content(xml_node_t *node);
void free_xml_buf(xml_buffer_t *buf);
void free_xml_node(xml_node_t* node);


#ifdef __cplusplus
}
#endif

#endif
