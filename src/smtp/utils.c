#include "smtp.h"

int split_arguments(char *buf, char **argv, int max)
{
    int argc = 0;
    char *word;

    while (*buf && argc < max) {

        // Skip tous les espaces et sauts à la ligne.
        while (*buf && (*buf == ' ' || *buf == '\r' || *buf == '\n'))
            buf++;

        // Vérifier si on est à la fin du texte, cas spécial "         " (plein d'espaces sans arguments)
        if (*buf == 0)
            break;

        // Sauvegarder le pointeur vers le début du mot.
        word = buf;

        // Skip le mot entier.
        while (*buf && (*buf == ' ' || *buf == '\r' || *buf == '\n') == false)
            buf++;

        // Cas spécial, comptabiliser le dernier argument sans buffer overflow
        if (*buf)
            *buf++ = 0;

        // Sauvegarder le pointeur pointant vers le début du mot dans la liste des mots splittés.
        argv[argc++] = word;
    }

    return (argc);
}

int get_next_power(int i)
{
    int p = 4096;

    while (p < i)
        p <<= 1;
    return (p);
}