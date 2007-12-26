
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaml.h>

#include <assert.h>
#include <limits.h>

/*
 * Memory management.
 */

YAML_DECLARE(void *)
yaml_malloc(size_t size);

YAML_DECLARE(void *)
yaml_realloc(void *ptr, size_t size);

YAML_DECLARE(void)
yaml_free(void *ptr);

YAML_DECLARE(yaml_char_t *)
yaml_strdup(const yaml_char_t *);

/*
 * Error management.
 */

#define ERROR_INIT(error,error_type)                                            \
    (memset(&(error), 0, sizeof(error)),                                        \
     (error).type = (error_type),                                               \
     0)

#define READING_ERROR_INIT(error,error_type,error_problem,error_offset,error_value) \
    (memset(&(error), 0, sizeof(error)),                                        \
     (error).type = (error_type),                                               \
     (error).data.reading.problem = (error_problem),                            \
     (error).data.reading.offset = (error_offset),                              \
     (error).data.reading.value = (error_value),                                \
     0)

#define LOADING_ERROR_INIT(error,error_type,error_problem,error_problem_mark)   \
    (memset(&(error), 0, sizeof(error)),                                        \
     (error).type = (error_type),                                               \
     (error).data.loading.context = NULL,                                       \
     (error).data.loading.context_mark.index = 0,                               \
     (error).data.loading.context_mark.line = 0,                                \
     (error).data.loading.context_mark.column = 0,                              \
     (error).data.loading.problem = (error_problem),                            \
     (error).data.loading.problem_mark = (error_problem_mark),                  \
     0)

#define LOADING_ERROR_WITH_CONTEXT_INIT(error,error_type,error_context,error_context_mark,error_problem,error_problem_mark) \
    (memset(&(error), 0, sizeof(error)),                                        \
     (error).type = (error_type),                                               \
     (error).data.loading.context = (error_context),                            \
     (error).data.loading.context_mark = (error_context_mark),                  \
     (error).data.loading.problem = (error_problem),                            \
     (error).data.loading.problem_mark = (error_problem_mark),                  \
     0)

#define WRITING_ERROR_INIT(error,error_type,error_problem,error_offset)         \
    (memset(&(error), 0, sizeof(error)),                                        \
     (error).type = (error_type),                                               \
     (error).data.writing.problem = (error_problem),                            \
     (error).data.writing.offset = (error_offset),                              \
     0)

#define DUMPING_ERROR_INIT(error,error_type,error_problem)                      \
    (memset(&(error), 0, sizeof(error)),                                        \
     (error).type = (error_type),                                               \
     (error).type.dumping.problem = (error_problem),                            \
     0)

#define MEMORY_ERROR_INIT(self)                                                 \
    ERROR_INIT((self)->error,YAML_MEMORY_ERROR)

#define READER_ERROR_INIT(self,problem,offset)                                  \
    READING_ERROR_INIT((self)->error,YAML_READER_ERROR,problem,offset,-1)

#define DECODER_ERROR_INIT(self,problem,offset,value)                           \
    READING_ERROR_INIT((self)->error,YAML_DECODER_ERROR,problem,offset,value)

#define SCANNER_ERROR_INIT(self,problem,problem_mark)                           \
    LOADING_ERROR_INIT((self)->error,YAML_SCANNER_ERROR,problem,problem_mark)

#define SCANNER_ERROR_WITH_CONTEXT_INIT(self,context,context_mark,problem,problem_mark) \
    LOADING_ERROR_WITH_CONTEXT_INIT((self)->error,YAML_SCANNER_ERROR,context,context_mark,problem,problem_mark)

#define PARSER_ERROR_INIT(self,problem,problem_mark)                            \
    LOADING_ERROR_INIT((self)->error,YAML_PARSER_ERROR,problem,problem_mark)

#define PARSER_ERROR_WITH_CONTEXT_INIT(self,context,context_mark,problem,problem_mark)  \
    LOADING_ERROR_WITH_CONTEXT_INIT((self)->error,YAML_PARSER_ERROR,context,context_mark,problem,problem_mark)

#define COMPOSER_ERROR_INIT(self,problem,problem_mark)                          \
    LOADING_ERROR_INIT((self)->error,YAML_COMPOSER_ERROR,problem,problem_mark)

#define COMPOSER_ERROR_WITH_CONTEXT_INIT(self,context,context_mark,problem,problem_mark)    \
    LOADING_ERROR_WITH_CONTEXT_INIT((self)->error,YAML_COMPOSER_ERROR,context,context_mark,problem,problem_mark)

#define WRITER_ERROR_INIT(self,problem,offset)                                  \
    WRITING_ERROR_INIT((self)->error,YAML_WRITER_ERROR,problem,offset)

#define EMITTER_ERROR_INIT(self,context,problem)                                \
    DUMPING_ERROR_INIT((self)->error,YAML_EMITTER_ERROR,problem)

#define SERIALIZER_ERROR_INIT(self,context)                                     \
    DUMPING_ERROR_INIT((self)->error,YAML_SERIALIZER_ERROR,problem)

/*
 * The size of the input raw buffer.
 */

#define RAW_INPUT_BUFFER_CAPACITY   16384

/*
 * The size of the input buffer.
 *
 * The input buffer should be large enough to hold the content of the raw
 * buffer after it is decoded.
 */

#define INPUT_BUFFER_CAPACITY   (RAW_INPUT_BUFFER_CAPACITY*3)

/*
 * The size of the output buffer.
 */

#define OUTPUT_BUFFER_CAPACITY  16384

/*
 * The size of the output raw buffer.
 *
 * The raw buffer should be able to hold the content of the output buffer
 * after it is encoded.
 */

#define RAW_OUTPUT_BUFFER_CAPACITY  (OUTPUT_BUFFER_CAPACITY*2+2)

/*
 * The size of other stacks and queues.
 */

#define INITIAL_STACK_CAPACITY  16
#define INITIAL_QUEUE_CAPACITY  16
#define INITIAL_STRING_CAPACITY 16

/*
 * Input/output buffer management.
 */

#define STORAGE_INIT(self,storage,storage_capacity)                             \
    (((storage).buffer = yaml_malloc(storage_capacity)) ?                       \
        ((storage).pointer = (storage).length = 0,                              \
         (buffer).capacity = (storage_capacity)                                 \
         1) :                                                                   \
        ((self)->error.type = YAML_MEMORY_ERROR,                                \
         0))

#define STORAGE_DEL(self,storage)                                               \
    (yaml_free((storage).buffer),                                               \
     (storage).pointer = (storage).length = (storage).capacity = 0)

/*
 * String management.
 */

typedef struct yaml_string_s {
    yaml_char_t *buffer;
    size_t pointer;
    size_t capacity;
} yaml_string_t;

YAML_DECLARE(int)
yaml_string_extend(yaml_char_t **buffer, size_t *capacity);

YAML_DECLARE(int)
yaml_string_join(
        yaml_char_t **base_buffer, size_t *base_pointer, size_t *base_capacity,
        yaml_char_t *adj_buffer, size_t adj_pointer, size_t adj_capacity);

#define NULL_STRING { NULL, NULL, NULL }

#define STRING(string,capacity)   { (string), 0, (capacity) }

#define STRING_INIT(self,string,string_capacity)                                \
    (((string).buffer = yaml_malloc(string_capacity)) ?                         \
        ((string).pointer = 0,                                                  \
         (string).capacity = (string_capacity),                                 \
         memset((string).buffer, 0, (string_capacity)),                         \
         1) :                                                                   \
        ((self)->error.type = YAML_MEMORY_ERROR,                                \
         0))

#define STRING_DEL(self,string)                                                 \
    (yaml_free((string).buffer),                                                \
     (string).buffer = NULL,                                                    \
     ((string).pointer = (string).capacity = 0))

#define STRING_EXTEND(self,string)                                              \
    ((((string).pointer+5 < (string).capacity)                                  \
        || yaml_string_extend(&(string).buffer, &(string).capacity)) ?          \
     1 :                                                                        \
     ((self)->error.type = YAML_MEMORY_ERROR,                                   \
      0))

#define CLEAR(self,string)                                                      \
    ((string).pointer = 0,                                                      \
     memset((string).buffer, 0, (string).capacity))

#define JOIN(self,base_string,adj_string)                                       \
    ((yaml_string_join(&(base_string).buffer, &(base_string).pointer,           \
                       &(base_string).capacity, (adj_string).buffer,            \
                       (adj_string).pointer, (adj_string).capacity)) ?          \
        ((adj_string).pointer = 0,                                              \
         1) :                                                                   \
        ((self)->error.type = YAML_MEMORY_ERROR,                                \
         0))

/*
 * String check operations.
 */

/*
 * Get the octet at the specified position.
 */

#define OCTET_AT(string,offset)                                                 \
    ((string).buffer[(string).pointer+(offset)])

/*
 * Get the current offset.
 */

#define OCTET(string)   OCTET_AT((string),0)

/*
 * Check the octet at the specified position.
 */

#define CHECK_AT(string,octet,offset)                                           \
    (OCTET_AT((string),(offset)) == (yaml_char_t)(octet))

/*
 * Check the current octet in the buffer.
 */

#define CHECK(string,octet) CHECK_AT((string),(octet),0)

/*
 * Check if the character at the specified position is an alphabetical
 * character, a digit, '_', or '-'.
 */

#define IS_ALPHA_AT(string,offset)                                              \
     ((OCTET_AT((string),(offset)) >= (yaml_char_t) '0' &&                      \
       OCTET_AT((string),(offset)) <= (yaml_char_t) '9') ||                     \
      (OCTET_AT((string),(offset)) >= (yaml_char_t) 'A' &&                      \
       OCTET_AT((string),(offset)) <= (yaml_char_t) 'Z') ||                     \
      (OCTET_AT((string),(offset)) >= (yaml_char_t) 'a' &&                      \
       OCTET_AT((string),(offset)) <= (yaml_char_t) 'z') ||                     \
      OCTET_AT((string),(offset)) == '_' ||                                     \
      OCTET_AT((string),(offset)) == '-')

#define IS_ALPHA(string)    IS_ALPHA_AT((string),0)

/*
 * Check if the character at the specified position is a digit.
 */

#define IS_DIGIT_AT(string,offset)                                              \
     ((OCTET_AT((string),(offset)) >= (yaml_char_t) '0' &&                      \
       OCTET_AT((string),(offset)) <= (yaml_char_t) '9'))

#define IS_DIGIT(string)    IS_DIGIT_AT((string),0)

/*
 * Get the value of a digit.
 */

#define AS_DIGIT_AT(string,offset)                                              \
     (OCTET_AT((string),(offset)) - (yaml_char_t) '0')

#define AS_DIGIT(string)    AS_DIGIT_AT((string),0)

/*
 * Check if the character at the specified position is a hex-digit.
 */

#define IS_HEX_AT(string,offset)                                                \
     ((OCTET_AT((string),(offset)) >= (yaml_char_t) '0' &&                      \
       OCTET_AT((string),(offset)) <= (yaml_char_t) '9') ||                     \
      (OCTET_AT((string),(offset)) >= (yaml_char_t) 'A' &&                      \
       OCTET_AT((string),(offset)) <= (yaml_char_t) 'F') ||                     \
      (OCTET_AT((string),(offset)) >= (yaml_char_t) 'a' &&                      \
       OCTET_AT((string),(offset)) <= (yaml_char_t) 'f'))

#define IS_HEX(string)    IS_HEX_AT((string),0)

/*
 * Get the value of a hex-digit.
 */

#define AS_HEX_AT(string,offset)                                                \
      ((OCTET_AT((string),(offset)) >= (yaml_char_t) 'A' &&                     \
        OCTET_AT((string),(offset)) <= (yaml_char_t) 'F') ?                     \
       (OCTET_AT((string),(offset)) - (yaml_char_t) 'A' + 10) :                 \
       (OCTET_AT((string),(offset)) >= (yaml_char_t) 'a' &&                     \
        OCTET_AT((string),(offset)) <= (yaml_char_t) 'f') ?                     \
       (OCTET_AT((string),(offset)) - (yaml_char_t) 'a' + 10) :                 \
       (OCTET_AT((string),(offset)) - (yaml_char_t) '0'))
 
#define AS_HEX(string)  AS_HEX_AT((string),0)
 
/*
 * Check if the character is ASCII.
 */

#define IS_ASCII_AT(string,offset)                                              \
    (OCTET_AT((string),(offset)) <= (yaml_char_t) '\x7F')

#define IS_ASCII(string)    IS_ASCII_AT((string),0)

/*
 * Check if the character can be printed unescaped.
 */

#define IS_PRINTABLE_AT(string,offset)                                          \
    ((OCTET_AT((string),(offset)) == 0x0A)          /* . == #x0A */             \
     || (OCTET_AT((string),(offset)) >= 0x20        /* #x20 <= . <= #x7E */     \
         && OCTET_AT((string),(offset)) <= 0x7E)                                \
     || (OCTET_AT((string),(offset)) == 0xC2        /* #0xA0 <= . <= #xD7FF */  \
         && OCTET_AT((string),(offset)+1) >= 0xA0)                              \
     || (OCTET_AT((string),(offset)) > 0xC2                                     \
         && OCTET_AT((string),(offset)) < 0xED)                                 \
     || (OCTET_AT((string),(offset)) == 0xED                                    \
         && OCTET_AT((string),(offset)+1) < 0xA0)                               \
     || (OCTET_AT((string),(offset)) == 0xEE)                                   \
     || (OCTET_AT((string),(offset)) == 0xEF        /* #xE000 <= . <= #xFFFD */ \
         && !(OCTET_AT((string),(offset)+1) == 0xBB        /* && . != #xFEFF */ \
             && OCTET_AT((string),(offset)+2) == 0xBF)                          \
         && !(OCTET_AT((string),(offset)+1) == 0xBF                             \
             && (OCTET_AT((string),(offset)+2) == 0xBE                          \
                 || OCTET_AT((string),(offset)+2) == 0xBF))))

#define IS_PRINTABLE(string)    IS_PRINTABLE_AT((string),0)

/*
 * Check if the character at the specified position is NUL.
 */

#define IS_Z_AT(string,offset)    CHECK_AT((string),'\0',(offset))

#define IS_Z(string)    IS_Z_AT((string),0)

/*
 * Check if the character at the specified position is BOM.
 */

#define IS_BOM_AT(string,offset)                                                \
     (CHECK_AT((string),'\xEF',(offset))                                        \
      && CHECK_AT((string),'\xBB',(offset)+1)                                   \
      && CHECK_AT((string),'\xBF',(offset)+2))  /* BOM (#xFEFF) */

#define IS_BOM(string)  IS_BOM_AT(string,0)

/*
 * Check if the character at the specified position is space.
 */

#define IS_SPACE_AT(string,offset)  CHECK_AT((string),' ',(offset))

#define IS_SPACE(string)    IS_SPACE_AT((string),0)

/*
 * Check if the character at the specified position is tab.
 */

#define IS_TAB_AT(string,offset)    CHECK_AT((string),'\t',(offset))

#define IS_TAB(string)  IS_TAB_AT((string),0)

/*
 * Check if the character at the specified position is blank (space or tab).
 */

#define IS_BLANK_AT(string,offset)                                              \
    (IS_SPACE_AT((string),(offset)) || IS_TAB_AT((string),(offset)))

#define IS_BLANK(string)    IS_BLANK_AT((string),0)

/*
 * Check if the character at the specified position is a line break.
 */

#define IS_BREAK_AT(string,offset)                                              \
    (CHECK_AT((string),'\r',(offset))               /* CR (#xD)*/               \
     || CHECK_AT((string),'\n',(offset))            /* LF (#xA) */              \
     || (CHECK_AT((string),'\xC2',(offset))                                     \
         && CHECK_AT((string),'\x85',(offset)+1))   /* NEL (#x85) */            \
     || (CHECK_AT((string),'\xE2',(offset))                                     \
         && CHECK_AT((string),'\x80',(offset)+1)                                \
         && CHECK_AT((string),'\xA8',(offset)+2))   /* LS (#x2028) */           \
     || (CHECK_AT((string),'\xE2',(offset))                                     \
         && CHECK_AT((string),'\x80',(offset)+1)                                \
         && CHECK_AT((string),'\xA9',(offset)+2)))  /* PS (#x2029) */

#define IS_BREAK(string)    IS_BREAK_AT((string),0)

#define IS_CRLF_AT(string,offset)                                               \
     (CHECK_AT((string),'\r',(offset)) && CHECK_AT((string),'\n',(offset)+1))

#define IS_CRLF(string) IS_CRLF_AT((string),0)

/*
 * Check if the character is a line break or NUL.
 */

#define IS_BREAKZ_AT(string,offset)                                             \
    (IS_BREAK_AT((string),(offset)) || IS_Z_AT((string),(offset)))

#define IS_BREAKZ(string)   IS_BREAKZ_AT((string),0)

/*
 * Check if the character is a line break, space, or NUL.
 */

#define IS_SPACEZ_AT(string,offset)                                             \
    (IS_SPACE_AT((string),(offset)) || IS_BREAKZ_AT((string),(offset)))

#define IS_SPACEZ(string)   IS_SPACEZ_AT((string),0)

/*
 * Check if the character is a line break, space, tab, or NUL.
 */

#define IS_BLANKZ_AT(string,offset)                                             \
    (IS_BLANK_AT((string),(offset)) || IS_BREAKZ_AT((string),(offset)))

#define IS_BLANKZ(string)   IS_BLANKZ_AT((string),0)

/*
 * Determine the width of the character.
 */

#define WIDTH_AT(string,offset)                                                 \
     ((OCTET_AT((string),(offset)) & 0x80) == 0x00 ? 1 :                        \
      (OCTET_AT((string),(offset)) & 0xE0) == 0xC0 ? 2 :                        \
      (OCTET_AT((string),(offset)) & 0xF0) == 0xE0 ? 3 :                        \
      (OCTET_AT((string),(offset)) & 0xF8) == 0xF0 ? 4 : 0)

#define WIDTH(string)   WIDTH_AT((string),0)

/*
 * Move the string pointer to the next character.
 */

#define MOVE(string)    ((string).pointer += WIDTH((string)))

/*
 * Copy a single octet and bump the pointers.
 */

#define COPY_OCTET(string_a,string_b)                                           \
    ((string_a).buffer[(string_a).pointer++]                                    \
     = (string_b).buffer[(string_b).pointer++])

/*
 * Copy a character and move the pointers of both strings.
 */

#define COPY(string_a,string_b)                                                 \
    ((OCTET(string_b) & 0x80) == 0x00 ?                                         \
     COPY_OCTET((string_a),(string_b)) :                                        \
     (OCTET(string_b) & 0xE0) == 0xC0 ?                                         \
     (COPY_OCTET((string_a),(string_b)),                                        \
      COPY_OCTET((string_a),(string_b))) :                                      \
     (OCTET(string_b) & 0xF0) == 0xE0 ?                                         \
     (COPY_OCTET((string_a),(string_b)),                                        \
      COPY_OCTET((string_a),(string_b)),                                        \
      COPY_OCTET((string_a),(string_b))) :                                      \
     (OCTET(string_b) & 0xF8) == 0xF0 ?                                         \
     (COPY_OCTET((string_a),(string_b)),                                        \
      COPY_OCTET((string_a),(string_b)),                                        \
      COPY_OCTET((string_a),(string_b)),                                        \
      COPY_OCTET((string_a),(string_b))) : 0)                                   \

/*
 * Stack and queue management.
 */

YAML_DECLARE(int)
yaml_stack_extend(void **list, size_t size, size_t *length, size_t *capacity);

YAML_DECLARE(int)
yaml_queue_extend(void **list, size_t size,
        size_t *head, size_t *tail, size_t *capacity);

#define STACK_INIT(self,stack,stack_capacity)                                   \
    (((stack).list = yaml_malloc((stack_capacity)*sizeof(*(stack).list))) ?     \
        ((stack).length = 0,                                                    \
         (stack).capacity = (stack_capacity),                                   \
         1) :                                                                   \
        ((self)->error.type = YAML_MEMORY_ERROR,                                \
         0))

#define STACK_DEL(self,stack)                                                   \
    (yaml_free((stack).list),                                                   \
     (stack).list = NULL,                                                       \
     (stack).length = (stack).capacity = 0)

#define STACK_EMPTY(self,stack)                                                 \
    ((stack).length == 0)

#define PUSH(self,stack,value)                                                  \
    (((stack).length < (stack).capacity                                         \
      || yaml_stack_extend((void **)&(stack).list, sizeof(*(stack).list),       \
              &(stack).length, &(stack).capacity)) ?                            \
        ((stack).list[(stack).length++] = (value),                              \
         1) :                                                                   \
        ((self)->error.type = YAML_MEMORY_ERROR,                                \
         0))

#define POP(self,stack)                                                         \
    ((stack).list[--(stack).length])

#define QUEUE_INIT(self,queue,queue_capacity)                                   \
    (((queue).list = yaml_malloc((queue_capacity)*sizeof(*(queue).list))) ?     \
        ((queue).head = (queue).tail = 0,                                       \
         (queue).capacity = (queue_capacity),                                   \
         1) :                                                                   \
        ((self)->error.type = YAML_MEMORY_ERROR,                                \
         0))

#define QUEUE_DEL(self,queue)                                                   \
    (yaml_free((queue).list),                                                   \
     (queue).list = NULL,                                                       \
     (queue).head = (queue).tail = (queue).capacity = 0)

#define QUEUE_EMPTY(self,queue)                                                 \
    ((queue).head == (queue).tail)

#define ENQUEUE(self,queue,value)                                               \
    (((queue).tail != (queue).capacity                                          \
      || yaml_queue_extend((void **)&(queue).list, sizeof(*(queue).list),       \
          &(queue).head, &(queue).tail, &(queue).capacity)) ?                   \
        ((queue).list[(queue).tail++] = (value),                                \
         1) :                                                                   \
        ((self)->error.type = YAML_MEMORY_ERROR,                                \
         0))

#define DEQUEUE(self,queue)                                                     \
    ((queue).list[(queue).head++])

#define QUEUE_INSERT(self,queue,index,value)                                    \
    (((queue).tail != (queue).capacity                                          \
      || yaml_queue_extend((void **)&(queue).list, sizeof(*(queue).list),       \
          &(queue).head, &(queue).tail, &(queue).capacity)) ?                   \
        (memmove((queue).list+(queue).head+(index)+1,                           \
                 (queue).list+(queue).head+(index),                             \
            ((queue).tail-(queue).head-(index))*sizeof(*(queue).list)),         \
         (queue).list[(queue).head+(index)] = (value),                          \
         (queue).tail++,                                                        \
         1) :                                                                   \
        ((self)->error.type = YAML_MEMORY_ERROR,                                \
         0))

/*
 * Token initializers.
 */

#define TOKEN_INIT(token,token_type,token_start_mark,token_end_mark)            \
    (memset(&(token), 0, sizeof(yaml_token_t)),                                 \
     (token).type = (token_type),                                               \
     (token).start_mark = (token_start_mark),                                   \
     (token).end_mark = (token_end_mark))

#define STREAM_START_TOKEN_INIT(token,token_encoding,start_mark,end_mark)       \
    (TOKEN_INIT((token),YAML_STREAM_START_TOKEN,(start_mark),(end_mark)),       \
     (token).data.stream_start.encoding = (token_encoding))

#define STREAM_END_TOKEN_INIT(token,start_mark,end_mark)                        \
    (TOKEN_INIT((token),YAML_STREAM_END_TOKEN,(start_mark),(end_mark)))

#define ALIAS_TOKEN_INIT(token,token_value,start_mark,end_mark)                 \
    (TOKEN_INIT((token),YAML_ALIAS_TOKEN,(start_mark),(end_mark)),              \
     (token).data.alias.value = (token_value))

#define ANCHOR_TOKEN_INIT(token,token_value,start_mark,end_mark)                \
    (TOKEN_INIT((token),YAML_ANCHOR_TOKEN,(start_mark),(end_mark)),             \
     (token).data.anchor.value = (token_value))

#define TAG_TOKEN_INIT(token,token_handle,token_suffix,start_mark,end_mark)     \
    (TOKEN_INIT((token),YAML_TAG_TOKEN,(start_mark),(end_mark)),                \
     (token).data.tag.handle = (token_handle),                                  \
     (token).data.tag.suffix = (token_suffix))

#define SCALAR_TOKEN_INIT(token,token_value,token_length,token_style,start_mark,end_mark)   \
    (TOKEN_INIT((token),YAML_SCALAR_TOKEN,(start_mark),(end_mark)),             \
     (token).data.scalar.value = (token_value),                                 \
     (token).data.scalar.length = (token_length),                               \
     (token).data.scalar.style = (token_style))

#define VERSION_DIRECTIVE_TOKEN_INIT(token,token_major,token_minor,start_mark,end_mark)     \
    (TOKEN_INIT((token),YAML_VERSION_DIRECTIVE_TOKEN,(start_mark),(end_mark)),  \
     (token).data.version_directive.major = (token_major),                      \
     (token).data.version_directive.minor = (token_minor))

#define TAG_DIRECTIVE_TOKEN_INIT(token,token_handle,token_prefix,start_mark,end_mark)       \
    (TOKEN_INIT((token),YAML_TAG_DIRECTIVE_TOKEN,(start_mark),(end_mark)),      \
     (token).data.tag_directive.handle = (token_handle),                        \
     (token).data.tag_directive.prefix = (token_prefix))

/*
 * Event initializers.
 */

#define EVENT_INIT(event,event_type,event_start_mark,event_end_mark)            \
    (memset(&(event), 0, sizeof(yaml_event_t)),                                 \
     (event).type = (event_type),                                               \
     (event).start_mark = (event_start_mark),                                   \
     (event).end_mark = (event_end_mark))

#define STREAM_START_EVENT_INIT(event,event_encoding,start_mark,end_mark)       \
    (EVENT_INIT((event),YAML_STREAM_START_EVENT,(start_mark),(end_mark)),       \
     (event).data.stream_start.encoding = (event_encoding))

#define STREAM_END_EVENT_INIT(event,start_mark,end_mark)                        \
    (EVENT_INIT((event),YAML_STREAM_END_EVENT,(start_mark),(end_mark)))

#define DOCUMENT_START_EVENT_INIT(event,event_version_directive,                \
        event_tag_directives_list,event_tag_directives_length,                  \
        event_tag_directives_capacity,event_is_implicit,start_mark,end_mark)    \
    (EVENT_INIT((event),YAML_DOCUMENT_START_EVENT,(start_mark),(end_mark)),     \
     (event).data.document_start.version_directive = (event_version_directive), \
     (event).data.document_start.tag_directives.list = (event_tag_directives_list), \
     (event).data.document_start.tag_directives.length = (event_tag_directives_length), \
     (event).data.document_start.tag_directives.capacity = (event_tag_directives_capacity), \
     (event).data.document_start.is_implicit = (event_is_implicit))

#define DOCUMENT_END_EVENT_INIT(event,event_is_implicit,start_mark,end_mark)    \
    (EVENT_INIT((event),YAML_DOCUMENT_END_EVENT,(start_mark),(end_mark)),       \
     (event).data.document_end.is_implicit = (event_is_implicit))

#define ALIAS_EVENT_INIT(event,event_anchor,start_mark,end_mark)                \
    (EVENT_INIT((event),YAML_ALIAS_EVENT,(start_mark),(end_mark)),              \
     (event).data.alias.anchor = (event_anchor))

#define SCALAR_EVENT_INIT(event,event_anchor,event_tag,event_value,             \
        event_length,event_is_plain_implicit,event_is_quoted_implicit,          \
        event_style,start_mark,end_mark)                                        \
    (EVENT_INIT((event),YAML_SCALAR_EVENT,(start_mark),(end_mark)),             \
     (event).data.scalar.anchor = (event_anchor),                               \
     (event).data.scalar.tag = (event_tag),                                     \
     (event).data.scalar.value = (event_value),                                 \
     (event).data.scalar.length = (event_length),                               \
     (event).data.scalar.is_plain_implicit = (event_is_plain_implicit),         \
     (event).data.scalar.is_quoted_implicit = (event_is_quoted_implicit),       \
     (event).data.scalar.style = (event_style))

#define SEQUENCE_START_EVENT_INIT(event,event_anchor,event_tag,                 \
        event_is_implicit,event_style,start_mark,end_mark)                      \
    (EVENT_INIT((event),YAML_SEQUENCE_START_EVENT,(start_mark),(end_mark)),     \
     (event).data.sequence_start.anchor = (event_anchor),                       \
     (event).data.sequence_start.tag = (event_tag),                             \
     (event).data.sequence_start.is_implicit = (event_is_implicit),             \
     (event).data.sequence_start.style = (event_style))

#define SEQUENCE_END_EVENT_INIT(event,start_mark,end_mark)                      \
    (EVENT_INIT((event),YAML_SEQUENCE_END_EVENT,(start_mark),(end_mark)))

#define MAPPING_START_EVENT_INIT(event,event_anchor,event_tag,                  \
        event_is_implicit,event_style,start_mark,end_mark)                      \
    (EVENT_INIT((event),YAML_MAPPING_START_EVENT,(start_mark),(end_mark)),      \
     (event).data.mapping_start.anchor = (event_anchor),                        \
     (event).data.mapping_start.tag = (event_tag),                              \
     (event).data.mapping_start.is_implicit = (event_is_implicit),              \
     (event).data.mapping_start.style = (event_style))

#define MAPPING_END_EVENT_INIT(event,start_mark,end_mark)                       \
    (EVENT_INIT((event),YAML_MAPPING_END_EVENT,(start_mark),(end_mark)))

/*
 * Document initializer.
 */

#define DOCUMENT_INIT(document,document_nodes_start,document_nodes_end,         \
        document_version_directive,document_tag_directives_start,               \
        document_tag_directives_end,document_start_implicit,                    \
        document_end_implicit,document_start_mark,document_end_mark)            \
    (memset(&(document), 0, sizeof(yaml_document_t)),                           \
     (document).nodes.start = (document_nodes_start),                           \
     (document).nodes.end = (document_nodes_end),                               \
     (document).nodes.top = (document_nodes_start),                             \
     (document).version_directive = (document_version_directive),               \
     (document).tag_directives.start = (document_tag_directives_start),         \
     (document).tag_directives.end = (document_tag_directives_end),             \
     (document).start_implicit = (document_start_implicit),                     \
     (document).end_implicit = (document_end_implicit),                         \
     (document).start_mark = (document_start_mark),                             \
     (document).end_mark = (document_end_mark))

/*
 * Node initializers.
 */

#define NODE_INIT(node,node_type,node_tag,node_start_mark,node_end_mark)        \
    (memset(&(node), 0, sizeof(yaml_node_t)),                                   \
     (node).type = (node_type),                                                 \
     (node).tag = (node_tag),                                                   \
     (node).start_mark = (node_start_mark),                                     \
     (node).end_mark = (node_end_mark))

#define SCALAR_NODE_INIT(node,node_tag,node_value,node_length,                  \
        node_style,start_mark,end_mark)                                         \
    (NODE_INIT((node),YAML_SCALAR_NODE,(node_tag),(start_mark),(end_mark)),     \
     (node).data.scalar.value = (node_value),                                   \
     (node).data.scalar.length = (node_length),                                 \
     (node).data.scalar.style = (node_style))

#define SEQUENCE_NODE_INIT(node,node_tag,node_items_start,node_items_end,       \
        node_style,start_mark,end_mark)                                         \
    (NODE_INIT((node),YAML_SEQUENCE_NODE,(node_tag),(start_mark),(end_mark)),   \
     (node).data.sequence.items.start = (node_items_start),                     \
     (node).data.sequence.items.end = (node_items_end),                         \
     (node).data.sequence.items.top = (node_items_start),                       \
     (node).data.sequence.style = (node_style))

#define MAPPING_NODE_INIT(node,node_tag,node_pairs_start,node_pairs_end,        \
        node_style,start_mark,end_mark)                                         \
    (NODE_INIT((node),YAML_MAPPING_NODE,(node_tag),(start_mark),(end_mark)),    \
     (node).data.mapping.pairs.start = (node_pairs_start),                      \
     (node).data.mapping.pairs.end = (node_pairs_end),                          \
     (node).data.mapping.pairs.top = (node_pairs_start),                        \
     (node).data.mapping.style = (node_style))

/*
 * This structure holds information about a potential simple key.
 */

typedef struct yaml_simple_key_s {
    /* Is a simple key possible? */
    int is_possible;
    /* Is a simple key required? */
    int is_required;
    /* The number of the token. */
    size_t token_number;
    /* The position mark. */
    yaml_mark_t mark;
} yaml_simple_key_t;

/*
 * The states of the parser.
 */

typedef enum yaml_parser_state_e {
    /* Expect STREAM-START. */
    YAML_PARSE_STREAM_START_STATE,
    /* Expect the beginning of an implicit document. */
    YAML_PARSE_IMPLICIT_DOCUMENT_START_STATE,
    /* Expect DOCUMENT-START. */
    YAML_PARSE_DOCUMENT_START_STATE,
    /* Expect the content of a document. */
    YAML_PARSE_DOCUMENT_CONTENT_STATE,
    /* Expect DOCUMENT-END. */
    YAML_PARSE_DOCUMENT_END_STATE,
    /* Expect a block node. */
    YAML_PARSE_BLOCK_NODE_STATE,
    /* Expect a block node or indentless sequence. */
    YAML_PARSE_BLOCK_NODE_OR_INDENTLESS_SEQUENCE_STATE,
    /* Expect a flow node. */
    YAML_PARSE_FLOW_NODE_STATE,
    /* Expect the first entry of a block sequence. */
    YAML_PARSE_BLOCK_SEQUENCE_FIRST_ENTRY_STATE,
    /* Expect an entry of a block sequence. */
    YAML_PARSE_BLOCK_SEQUENCE_ENTRY_STATE,
    /* Expect an entry of an indentless sequence. */
    YAML_PARSE_INDENTLESS_SEQUENCE_ENTRY_STATE,
    /* Expect the first key of a block mapping. */
    YAML_PARSE_BLOCK_MAPPING_FIRST_KEY_STATE,
    /* Expect a block mapping key. */
    YAML_PARSE_BLOCK_MAPPING_KEY_STATE,
    /* Expect a block mapping value. */
    YAML_PARSE_BLOCK_MAPPING_VALUE_STATE,
    /* Expect the first entry of a flow sequence. */
    YAML_PARSE_FLOW_SEQUENCE_FIRST_ENTRY_STATE,
    /* Expect an entry of a flow sequence. */
    YAML_PARSE_FLOW_SEQUENCE_ENTRY_STATE,
    /* Expect a key of an ordered mapping. */
    YAML_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_KEY_STATE,
    /* Expect a value of an ordered mapping. */
    YAML_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_VALUE_STATE,
    /* Expect the and of an ordered mapping entry. */
    YAML_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_END_STATE,
    /* Expect the first key of a flow mapping. */
    YAML_PARSE_FLOW_MAPPING_FIRST_KEY_STATE,
    /* Expect a key of a flow mapping. */
    YAML_PARSE_FLOW_MAPPING_KEY_STATE,
    /* Expect a value of a flow mapping. */
    YAML_PARSE_FLOW_MAPPING_VALUE_STATE,
    /* Expect an empty value of a flow mapping. */
    YAML_PARSE_FLOW_MAPPING_EMPTY_VALUE_STATE,
    /* Expect nothing. */
    YAML_PARSE_END_STATE
} yaml_parser_state_t;

/*
 * This structure holds aliases data.
 */

typedef struct yaml_alias_data_s {
    /* The anchor. */
    yaml_char_t *anchor;
    /* The node id. */
    int index;
    /* The anchor mark. */
    yaml_mark_t mark;
} yaml_alias_data_t;

/*
 * The structure that holds data used by the file and string readers.
 */

typedef union yaml_standard_reader_data_u {
    /* String input data. */
    yaml_string_t string;
    /* File input data. */
    FILE *file;
} yaml_standard_reader_data_t;

/*
 * The internal parser structure.
 */

struct yaml_parser_s {

    /*
     * Error stuff.
     */

    yaml_error_t error;

    /*
     * Reader stuff.
     */

    /* The read handler. */
    yaml_reader_t *reader;

    /* The application data to be passed to the reader. */
    void *reader_data;

    /* Standard (string or file) input data. */
    yaml_standard_reader_data_t standard_reader_data;

    /* EOF flag. */
    int is_eof;

    /* The working buffer. */
    struct {
        yaml_char_t *buffer;
        size_t pointer;
        size_t length;
        size_t capacity;
    } input;

    /* The number of unread characters in the buffer. */
    size_t unread;

    /* The raw buffer. */
    struct {
        unsigned char *buffer;
        size_t pointer;
        size_t length;
        size_t capacity;
    } raw_input;

    /* The input encoding. */
    yaml_encoding_t encoding;

    /* The offset of the current position (in bytes). */
    size_t offset;

    /* The mark of the current position. */
    yaml_mark_t mark;

    /*
     * Scanner stuff.
     */

    /* Have we started to scan the input stream? */
    int is_stream_start_produced;

    /* Have we reached the end of the input stream? */
    int is_stream_end_produced;

    /* The number of unclosed '[' and '{' indicators. */
    int flow_level;

    /* The tokens queue. */
    struct {
        yaml_token_t *list;
        size_t head;
        size_t tail;
        size_t capacity;
    } tokens;

    /* The number of tokens fetched from the queue. */
    size_t tokens_parsed;

    /* Does the tokens queue contain a token ready for dequeueing. */
    int is_token_available;

    /* The indentation levels stack. */
    struct {
        int *list;
        size_t length;
        size_t capacity;
    } indents;

    /* The current indentation level. */
    int indent;

    /* May a simple key occur at the current position? */
    int is_simple_key_allowed;

    /* The stack of simple keys. */
    struct {
        yaml_simple_key_t *list;
        size_t length;
        size_t capacity;
    } simple_keys;

    /*
     * Parser stuff.
     */

    /* The parser states stack. */
    struct {
        yaml_parser_state_t *list;
        size_t length;
        size_t capacity;
    } states;

    /* The current parser state. */
    yaml_parser_state_t state;

    /* The stack of marks. */
    struct {
        yaml_mark_t *list;
        size_t length;
        size_t capacity;
    } marks;

    /* The list of TAG directives. */
    struct {
        yaml_tag_directive_t *list;
        size_t length;
        size_t capacity;
    } tag_directives;

    /*
     * Dumper stuff.
     */

    /* The resolve handler. */
    yaml_resolver_t *resolver;

    /* The application data to be passed to the resolver. */
    void *resolver_data;

    /* The alias data. */
    struct {
        yaml_alias_data_t *list;
        size_t length;
        size_t capacity;
    } aliases;

    /* The currently parsed document. */
    yaml_document_t *document;

};

/*
 * The emitter states.
 */

typedef enum yaml_emitter_state_e {
    /** Expect STREAM-START. */
    YAML_EMIT_STREAM_START_STATE,
    /** Expect the first DOCUMENT-START or STREAM-END. */
    YAML_EMIT_FIRST_DOCUMENT_START_STATE,
    /** Expect DOCUMENT-START or STREAM-END. */
    YAML_EMIT_DOCUMENT_START_STATE,
    /** Expect the content of a document. */
    YAML_EMIT_DOCUMENT_CONTENT_STATE,
    /** Expect DOCUMENT-END. */
    YAML_EMIT_DOCUMENT_END_STATE,
    /** Expect the first item of a flow sequence. */
    YAML_EMIT_FLOW_SEQUENCE_FIRST_ITEM_STATE,
    /** Expect an item of a flow sequence. */
    YAML_EMIT_FLOW_SEQUENCE_ITEM_STATE,
    /** Expect the first key of a flow mapping. */
    YAML_EMIT_FLOW_MAPPING_FIRST_KEY_STATE,
    /** Expect a key of a flow mapping. */
    YAML_EMIT_FLOW_MAPPING_KEY_STATE,
    /** Expect a value for a simple key of a flow mapping. */
    YAML_EMIT_FLOW_MAPPING_SIMPLE_VALUE_STATE,
    /** Expect a value of a flow mapping. */
    YAML_EMIT_FLOW_MAPPING_VALUE_STATE,
    /** Expect the first item of a block sequence. */
    YAML_EMIT_BLOCK_SEQUENCE_FIRST_ITEM_STATE,
    /** Expect an item of a block sequence. */
    YAML_EMIT_BLOCK_SEQUENCE_ITEM_STATE,
    /** Expect the first key of a block mapping. */
    YAML_EMIT_BLOCK_MAPPING_FIRST_KEY_STATE,
    /** Expect the key of a block mapping. */
    YAML_EMIT_BLOCK_MAPPING_KEY_STATE,
    /** Expect a value for a simple key of a block mapping. */
    YAML_EMIT_BLOCK_MAPPING_SIMPLE_VALUE_STATE,
    /** Expect a value of a block mapping. */
    YAML_EMIT_BLOCK_MAPPING_VALUE_STATE,
    /** Expect nothing. */
    YAML_EMIT_END_STATE
} yaml_emitter_state_t;

/*
 * The structure that holds data used by the file and string readers.
 */

typedef union yaml_standard_writer_data_u {
    /* String output data. */
    yaml_string_t string;
    size_t *length;
    /* File output data. */
    FILE *file;
} yaml_standard_writer_data_t;

/*
 * The internals emitter structure.
 */

struct yaml_emitter_s {

    /*
     * Error stuff.
     */

    yaml_error_t error;

    /*
     * Writer stuff.
     */

    /* Write handler. */
    yaml_writer_t *writer;

    /* A pointer for passing to the white handler. */
    void *writer_data;

    /* Standard (string or file) output data. */
    yaml_standard_writer_data_t standard_writer_data;

    /* The working buffer. */
    struct {
        yaml_char_t *buffer;
        size_t pointer;
        size_t length;
        size_t capacity;
    } output;

    /* The raw buffer. */
    struct {
        yaml_char_t *buffer;
        size_t pointer;
        size_t length;
        size_t capacity;
    } raw_output;

    /* The stream encoding. */
    yaml_encoding_t encoding;

    /*
     * Emitter stuff.
     */

    /* If the output is in the canonical style? */
    int is_canonical;
    /* The number of indentation spaces. */
    int best_indent;
    /* The preferred width of the output lines. */
    int best_width;
    /* Allow unescaped non-ASCII characters? */
    int is_unicode;
    /* The preferred line break. */
    yaml_break_t line_break;

    /* The stack of states. */
    struct {
        yaml_emitter_state_t *list;
        size_t length;
        size_t capacity;
    } states;

    /* The current emitter state. */
    yaml_emitter_state_t state;

    /* The event queue. */
    struct {
        yaml_event_t *list;
        size_t head;
        size_t tail;
        size_t capacity;
    } events;

    /* The stack of indentation levels. */
    struct {
        int *list;
        size_t length;
        size_t capacity;
    } indents;

    /* The list of tag directives. */
    struct {
        yaml_tag_directive_t *list;
        size_t length;
        size_t capacity;
    } tag_directives;

    /* The current indentation level. */
    int indent;

    /* The current flow level. */
    int flow_level;

    /* Is it the document root context? */
    int is_root_context;
    /* Is it a sequence context? */
    int is_sequence_context;
    /* Is it a mapping context? */
    int is_mapping_context;
    /* Is it a simple mapping key context? */
    int is_simple_key_context;

    /* The current line. */
    int line;
    /* The current column. */
    int column;
    /* If the last character was a whitespace? */
    int whitespace;
    /* If the last character was an indentation character (' ', '-', '?', ':')? */
    int indention;

    /* Anchor analysis. */
    struct {
        /* The anchor value. */
        yaml_char_t *anchor;
        /* The anchor length. */
        size_t anchor_length;
        /* Is it an alias? */
        int is_alias;
    } anchor_data;

    /* Tag analysis. */
    struct {
        /* The tag handle. */
        yaml_char_t *handle;
        /* The tag handle length. */
        size_t handle_length;
        /* The tag suffix. */
        yaml_char_t *suffix;
        /* The tag suffix length. */
        size_t suffix_length;
    } tag_data;

    /* Scalar analysis. */
    struct {
        /* The scalar value. */
        yaml_char_t *value;
        /* The scalar length. */
        size_t length;
        /* Does the scalar contain line breaks? */
        int is_multiline;
        /* Can the scalar be expessed in the flow plain style? */
        int is_flow_plain_allowed;
        /* Can the scalar be expressed in the block plain style? */
        int is_block_plain_allowed;
        /* Can the scalar be expressed in the single quoted style? */
        int is_single_quoted_allowed;
        /* Can the scalar be expressed in the literal or folded styles? */
        int is_block_allowed;
        /* The output style. */
        yaml_scalar_style_t style;
    } scalar_data;

    /*
     * Dumper stuff.
     */

    /* If the stream was already opened? */
    int is_opened;
    /* If the stream was already closed? */
    int is_closed;

    /* The information associated with the document nodes. */
    struct {
        /* The number of references. */
        size_t references;
        /* The anchor id. */
        int anchor;
        /* If the node has been emitted? */
        int is_serialized;
    } *anchors;

    /* The last assigned anchor id. */
    int last_anchor_id;

    /* The currently emitted document. */
    yaml_document_t *document;

};

/*
 * Reader: Ensure that the buffer contains at least `length` characters.
 */

YAML_DECLARE(int)
yaml_parser_update_buffer(yaml_parser_t *parser, size_t length);

/*
 * Scanner: Ensure that the token stack contains at least one token ready.
 */

YAML_DECLARE(int)
yaml_parser_fetch_more_tokens(yaml_parser_t *parser);

