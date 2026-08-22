#include "utils.h" 
#include <time.h>

void hora_atual(char *saida, int tam_saida)
{
    time_t agora = time(NULL);
    struct tm *info = localtime(&agora);
    strftime(saida, (size_t)tam_saida, "%H:%M:%S", info);

}