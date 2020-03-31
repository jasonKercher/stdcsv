#include "util.h"
#include "csv.h"

///* Signal Handlers */
//static struct sigaction act;
///* sigset_t and sa_flags only set to shut up valgrind */
//static sigset_t vg_shutup = { {0} };
//static int _signalsReady = FALSE;
//
//
///**
// * Internal Structure
// */
//struct csv_read_internal {
//        struct csv_record* _record;
//        FILE* file;
//        char* buffer;
//        char appendBuffer[CSV_BUFFER_FACTOR];
//        uint fieldsAllocated;
//        uint delimLen;
//        int bufferSize;
//        int recordLen;
//
//        /* Statistics */
//        uint rows;
//        uint inlineBreaks;
//
//        /* Properties */
//        int normalOrg;
//};
//
//
///**
// * Internal prototypes
// */
//
///**
// * csv_determine_delimiter chooses between comma,
// * pipe or tab depending on which one is found most.
// * If none of these delimiters are found, use comma.
// * If reader->delimiter was set externally, simply
// * update reader->_internal->delimLen and return.
// */
//void csv_determine_delimiter(struct csv_reader* reader, const char* header);
//
///**
// * csv_getline is a simple wrapper for safegetline
// * that will allocate more buffer space in the event
// * that we overflow. The input is copied to
// * reader->_internal->buffer
// */
//void csv_getline(struct csv_reader* reader);
//
///**
// * csv_growrecord allocates the initial memory for a
// * struct csv_field. It also reallocs an additional fields
// * if fields is already allocated a pointer.
// */
//void csv_growrecord(struct csv_reader*);
//
///**
// * csv_parse_none takes a pointer to the beginning of
// * a field. *begin is searched for the next delimiter.
// *
// * Returns:
// *      - struct csv_field representing parsed field
// *      - NULL on failure
// */
//struct csv_field csv_parse_none(struct csv_reader*, char** begin);
//
///**
// * csv_parse_weak behaves the same as csv_parse_none
// * except that it expects the next field to be quoted.
// * *begin is searched for a terminating quote and
// * a delimiter.
// *
// * Returns:
// *      - struct csv_field representing parsed field
// *      - NULL on failure
// */
//struct csv_field csv_parse_weak(struct csv_reader*, char** begin);
//
///**
// * csv_parse_rfc4180 takes a pointer to the beginning of
// * a field. *begin is searched for the next delimiter
// * while we are not within text qualification. This is the
// * default quotes and most flexible/ideal.
// *
// * Returns:
// *      - struct csv_field representing parsed field
// *      - NULL on failure
// */
//struct csv_field csv_parse_rfc4180(struct csv_reader*, char** begin);
//
//
//void csv_lowerstandard(struct csv_reader* reader)
//{
//        if (!reader->failsafeMode || reader->_internal->file == stdin) {
//                switch(reader->quotes) {
//                case QUOTE_ALL:
//                case QUOTE_RFC4180:
//                        fprintf(stderr,
//                                "Line %d: RFC4180 Qualifier issue.\n",
//                                1 + reader->_internal->rows + reader->_internal->inlineBreaks);
//                        break;
//                case QUOTE_WEAK:
//                        fprintf(stderr,
//                                "Line %d: Qualifier issue.\n",
//                                1 + reader->_internal->rows + reader->_internal->inlineBreaks);
//                        break;
//                default:
//                        fputs("Unexpected Condition.\n", stderr);
//                }
//
//                exit(EXIT_FAILURE);
//        }
//
//        switch(reader->quotes) {
//        case QUOTE_ALL:
//        case QUOTE_RFC4180:
//                fprintf(stderr,
//                        "Line %d: Qualifier issue. RFC4180 quotes disabled.\n",
//                        1 + reader->_internal->rows + reader->_internal->inlineBreaks);
//                break;
//        case QUOTE_WEAK:
//                fprintf(stderr,
//                        "Line %d: Qualifier issue. Quotes disabled.\n",
//                        1 + reader->_internal->rows + reader->_internal->inlineBreaks);
//                break;
//        default:
//                fputs("Unexpected Condition.\n", stderr);
//                exit(EXIT_FAILURE);
//        }
//
//        --reader->quotes;
//        reader->_internal->rows = 0;
//        reader->_internal->inlineBreaks = 0;
//        fseek(reader->_internal->file, 0, SEEK_SET);
//}
//
//struct csv_record* csv_get_record(struct csv_reader* reader)
//{
//        csv_getline(reader);
//
//        /* Can be removed if we get rid of CSV_MAX_RECORD_SIZE */
//        if (reader->_internal->recordLen == -2) {
//                fprintf(stderr,"Buffer overflow on line %d.\n",
//                                reader->_internal->rows + reader->_internal->inlineBreaks);
//                exit(EXIT_FAILURE);
//        }
//
//        if (reader->_internal->recordLen == EOF) {
//                reader->_internal->fieldsAllocated = 0;
//                reader->normal = reader->_internal->normalOrg;
//                //csv_destroy(reader);
//                return NULL;
//        }
//
//        return csv_parse(reader, reader->_internal->buffer);
//}
//
//struct csv_record* csv_parse(struct csv_reader* reader, char* line)
//{
//        uint fieldIndex = 0;
//        struct csv_record* record = reader->_internal->_record;
//        if (!reader->_internal->fieldsAllocated && !reader->_internal->delimLen)
//                csv_determine_delimiter(reader, line);
//
//        while(line[0] != '\0') {
//                if (fieldIndex)
//                        line += reader->_internal->delimLen;
//                if (++(fieldIndex) > reader->_internal->fieldsAllocated)
//                        csv_growrecord(reader);
//
//                switch(reader->quotes) {
//                case QUOTE_ALL:
//                        /* Not seeing a point to implementing this for reading. */
//                case QUOTE_RFC4180:
//                        record->fields[fieldIndex-1] = csv_parse_rfc4180(reader, &line);
//                        break;
//                case QUOTE_WEAK:
//                        if (reader->quotes && line[0] == '"') { /** Implicit Fallthrough **/
//                                record->fields[fieldIndex-1] = csv_parse_weak(reader, &line);
//                                break;
//                        }
//                case QUOTE_NONE:
//                        record->fields[fieldIndex-1] = csv_parse_none(reader, &line);
//                        break;
//                }
//
//                if (!record->fields[fieldIndex-1].begin) {
//                        /** lowerstandard will exit outside of failsafe mode **/
//                        csv_lowerstandard(reader);
//                        record->size = CSV_RESET;
//                        return record;
//                }
//        }
//
//        if (reader->normal > 0) {
//                /* Append fields if we are short */
//                while ((uint) reader->normal > fieldIndex) {
//                        if (++(fieldIndex) > reader->_internal->fieldsAllocated)
//                                csv_growrecord(reader);
//                        record->fields[fieldIndex-1].length = 0;
//                }
//                fieldIndex = reader->normal;
//        }
//
//        if (reader->normal == CSV_NORMAL_OPEN)
//                reader->normal = reader->_internal->fieldsAllocated;
//
//        ++reader->_internal->rows;
//
//        record->size = fieldIndex;
//
//        return record;
//}
//
//struct csv_field csv_parse_rfc4180(struct csv_reader* reader, char** begin)
//{
//        uint delimI = 0;
//        uint qualified = FALSE;
//        uint lastWasQuote = FALSE;
//        uint i = 0;
//
//        struct csv_field field = {NULL, 0};
//
//        while (1) {
//                for (; (*begin)[i] != '\0' && delimI != reader->_internal->delimLen; ++i) {
//                        if (qualified) {
//                                qualified = ((*begin)[i] != '"');
//                                if (!qualified) {
//                                        removecharat(*begin, i--);
//                                        lastWasQuote = TRUE;
//                                }
//                        } else {
//                                if ((*begin)[i] == reader->delimiter[delimI])
//                                        ++delimI;
//                                else
//                                        delimI = ((*begin)[i] == reader->delimiter[0]) ? 1 : 0;
//
//                                if ( (qualified = ((*begin)[i] == '"') ) ) {
//                                        if (!lastWasQuote)
//                                                removecharat(*begin, i--);
//                                }
//
//                                lastWasQuote = FALSE;
//                        }
//                }
//                if ((*begin)[i] == '\0' && qualified) {
//                        int newLineLen = strlen(reader->inlineBreak);
//                        if (newLineLen + reader->_internal->recordLen >= reader->_internal->bufferSize)
//                                csv_increasebuffer(reader);
//
//                        strcat(reader->_internal->buffer, reader->inlineBreak);
//                        csv_appendlines(reader);
//                        if (reader->_internal->recordLen == EOF) /** TODO **/
//                                return field;
//                        strcat(reader->_internal->buffer, reader->_internal->appendBuffer);
//                        ++reader->_internal->inlineBreaks;
//                        continue;
//                }
//                break;
//        }
//
//        field.begin = *begin;
//        field.length = (delimI == reader->_internal->delimLen) ? i - reader->_internal->delimLen : i;
//        *begin += field.length;
//
//        return field;
//}
//
//struct csv_field csv_parse_weak(struct csv_reader* reader, char** begin)
//{
//        unsigned int delimI = 0;
//        unsigned int onQuote = FALSE;
//        unsigned int i = 1;
//
//        struct csv_field field = {NULL, 0};
//
//        while (1) {
//                for (; (*begin)[i] != '\0' && delimI != reader->_internal->delimLen; ++i) {
//                        if (onQuote && (*begin)[i] == reader->delimiter[delimI]) {
//                                ++delimI;
//                        } else {
//                                delimI = 0;
//                                onQuote = ((*begin)[i] == '"');
//                        }
//                }
//                if ((*begin)[i] == '\0' && !onQuote) {
//                        int newLineLen = strlen(reader->inlineBreak);
//                        if (newLineLen + reader->_internal->recordLen >= reader->_internal->bufferSize)
//                                csv_increasebuffer(reader);
//
//                        strcat(reader->_internal->buffer, reader->inlineBreak);
//                        csv_appendlines(reader);
//                        if (reader->_internal->recordLen == EOF) /** TODO **/
//                                return field;
//                        strcat(reader->_internal->buffer, reader->_internal->appendBuffer);
//                        ++reader->_internal->inlineBreaks;
//                        continue;
//                }
//                break;
//        }
//
//        field.begin = *begin + 1;
//        field.length = (delimI == reader->_internal->delimLen) ? i - 2 - reader->_internal->delimLen : i - 2;
//        *begin += field.length + 1;
//
//        return field;
//}
//
//struct csv_field csv_parse_none(struct csv_reader* reader, char** begin)
//{
//        unsigned int delimI = 0;
//        unsigned int i = 0;
//        struct csv_field field = {NULL, 0};
//
//        for (; (*begin)[i] != '\0' && delimI != reader->_internal->delimLen; ++i) {
//                if ((*begin)[i] == reader->delimiter[delimI])
//                        ++delimI;
//                else if (delimI != 0)
//                        delimI = ((*begin)[i] == reader->delimiter[0]) ? 1 : 0;
//        }
//
//        field.begin = *begin;
//        field.length = (delimI == reader->_internal->delimLen) ? i - reader->_internal->delimLen : i;
//        *begin += field.length;
//
//        return field;
//}
//
//void csv_determine_delimiter(struct csv_reader* reader, const char* header)
//{
//        uint delimLen = strlen(reader->delimiter);
//        if (delimLen) {
//                reader->_internal->delimLen = delimLen;
//                return;
//        }
//
//        const char* delims = ",|\t";
//        int i = 0;
//        int sel = 0;
//        int count = 0;
//        int maxCount = 0;
//        for (; i < 3; ++i) {
//                count = charcount(header, delims[i]);
//                if (count > maxCount) {
//                        sel = i;
//                        maxCount = count;
//                }
//        }
//
//        reader->delimiter[0] = delims[sel];
//        reader->delimiter[1] = '\0';
//        reader->_internal->delimLen = 1;
//}
//
//void csv_reader_open(struct csv_reader* reader, const char* fileName)
//{
//        if (fileName) {
//                reader->_internal->file = fopen(fileName, "r");
//                EXIT_IF(!reader->_internal->file, fileName);
//        } else {
//                reader->_internal->file = stdin;
//        }
//}
//
//struct csv_reader* csv_reader_new()
//{
//        struct csv_reader* reader = NULL;
//
//        MALLOC(reader, sizeof(*reader));
//        *reader = (struct csv_reader) {
//                NULL            /* internal */
//                ,""             /* delimiter */
//                ,"\n"           /* inlineBreak */
//                ,QUOTE_RFC4180  /* quotes */
//                ,0              /* Normal */
//                ,FALSE          /* failsafeMode */
//        };
//
//        MALLOC(reader->_internal, sizeof(*reader->_internal));
//        *reader->_internal = (struct csv_read_internal) {
//                NULL               /* internal record */
//                ,stdin             /* file */
//                ,NULL              /* buffer */
//                ,""                /* appendBuffer */
//                ,0                 /* fieldsAllocated; */
//                ,0                 /* delimLen */
//                ,CSV_BUFFER_FACTOR /* bufferSize */
//                ,0                 /* recordLen */
//                ,0                 /* rows */
//                ,0                 /* inlineBreaks */
//                ,0                 /* normalOrg */
//        };
//
//        MALLOC(reader->_internal->buffer, CSV_BUFFER_FACTOR);
//        MALLOC(reader->_internal->_record, sizeof(struct csv_record));
//
//        if (!_signalsReady) {
//                /** Attach signal handlers **/
//                act.sa_mask = vg_shutup;
//                act.sa_flags = 0;
//                act.sa_handler = cleanexit;
//                sigaction(SIGINT, &act, NULL);
//                sigaction(SIGQUIT, &act, NULL);
//                sigaction(SIGTERM, &act, NULL);
//                sigaction(SIGHUP, &act, NULL);
//
//                _signalsReady = TRUE;
//        }
//
//        //csv_growrecord(reader);
//
//        return reader;
//}
//
//void csv_growrecord(struct csv_reader* reader)
//{
//        ++reader->_internal->fieldsAllocated;
//        uint arraySize = reader->_internal->fieldsAllocated * sizeof(struct csv_field);
//        if (reader->_internal->fieldsAllocated > 1) {
//                REALLOC(reader->_internal->_record->fields, arraySize);
//        } else {
//                MALLOC(reader->_internal->_record->fields, arraySize);
//        }
//}
//
//void csv_reader_free(struct csv_reader* reader)
//{
//        FREE(reader->_internal->buffer);
//        FREE(reader->_internal->_record);
//        FREE(reader->_internal);
//        FREE(reader);
//}
//
