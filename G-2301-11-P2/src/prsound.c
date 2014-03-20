#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "sound.h"

main()
{
	char buf[256];
	register int i;
	FILE *f;

	sampleFormat(PA_SAMPLE_S16BE,2); /* Valor por defecto, no hace falta llamar a esta función */

	if(openRecord("prueba")) puts("error");
	if(openPlay("prueba")) puts("error");

	f=fopen("pruebasonido","w+b");
	puts("Grabando");
	for(i=0; i < 10000; ++i)
	{
		recordSound(buf,160);
		fwrite(buf,1,160,f);
	}
	fclose(f);
	closeRecord();

	f=fopen("pruebasonido","rb");
	puts("Reproduciendo");
	for(i=0; i < 10000; ++i)
	{
		fread(buf,1,160,f);
		playSound(buf,160);
	}
	fclose(f);
	closePlay();
}

