#ifndef PARSE_H_
#define PARSE_H_

bool skip_until_char(char expected, String *str, char **out);
char pop_char(String *str);
char get_char(String *str);
char peek_next_char(String *str);
void skip_spaces_and_breaks(String *str);

Image get_image_path(String *line);

#endif //PARSE_H_
