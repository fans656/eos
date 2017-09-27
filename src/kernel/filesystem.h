typedef struct FILE FILE;

void init_filesystem();

FILE* fopen(char* name);
void fclose(FILE* fp);

void fseek(FILE* fp, size_t pos);
size_t ftell(FILE* fp);

void fwrite(FILE* fp, void* data, size_t size);
void fread(FILE* fp, size_t size, void* data);

size_t fsize(char* name);
