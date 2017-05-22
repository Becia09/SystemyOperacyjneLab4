#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

int petla;

void oblugaSigint(int sygnal)
{
	//printf("pid aktualnego procesu: %d\n", (int)getpid());
	petla = 0;
}

void oblugaSigtstp(int sygnal)
{
	printf("w czasie dzialania programu wystapil blokowany sygnal Sigtstp\n");
}



int main(int argc, char **argv)
{

    if (argc == 1)
    {
        printf ("Brak argumentu\n");
        return(0);
    }

	int exitStatus;



	//***********************************zmiana obslugi sygnalu sigint i SIGTSTP
	sigset_t iset;
	struct sigaction act;
	sigemptyset(&iset);
	act.sa_handler = &oblugaSigint;
	act.sa_mask = iset;
	act.sa_flags = 0;
	sigaction(SIGINT, &act, NULL);			//przechwycenie sigint




	sigset_t iset3;
	struct sigaction act3;
	sigemptyset(&iset3);
	act3.sa_handler = &oblugaSigtstp;
	act3.sa_mask = iset3;
	act3.sa_flags = 0;
	sigaction(SIGTSTP, &act3, NULL);	//przechwycenie SIGTSTP

	sigset_t iset2;
	sigemptyset(&iset2);
	sigaddset(&iset2, SIGTSTP);
	sigprocmask(SIG_BLOCK, &iset2, NULL);		//blokowanie SIGTSTP



	//***********************************************************ucinanie argumentu do najblizszej potegi 2
    int dlugosc = strlen(argv[1]);
	if (argc == 2)
	{

		int potega = 0;
		while (dlugosc > 1)
		{
			dlugosc /= 2;
			//printf("dlugosc: %d\n", dlugosc);
			potega++;
		}

		dlugosc = 1;
		for (int i = 0; i < potega; i++)
		{
			dlugosc *= 2;
		}
	}


		//*********************************************************************wyznaczanie argumentow dzieci

	if (dlugosc > 1)
	{
		int pol = dlugosc / 2;

		//printf("argv dziecka1: ");
		char *argvDziecka1 = malloc(sizeof(char)*pol);
		
		for (int i = 0; i < pol; i++)
		{
			argvDziecka1[i] = argv[1][i];
			//printf("%c", argvDziecka1[i]);
		}
		
		char *argvDziecka2 = malloc(sizeof(char)*pol);
		int j = 0;
		for (int i = pol; i < dlugosc; i++)
		{
			argvDziecka2[j] = argv[1][i];
			j++;
		}
		int dziecko2 = strlen(argvDziecka2);
		int dziecko1 = strlen(argvDziecka1);


		//**************************************************************sklejanie argumentow rodzicow i dzieci
		int argument;
		if (argc == 2)
		{
			argument = 1;
		}
		else
		{
			argument = 2;
			dlugosc = strlen(argv[2]);
		}
		char *RodzicIdziecko1 = malloc(sizeof(char)*(dlugosc + pol + 1));
		char *RodzicIdziecko2 = malloc(sizeof(char)*(dlugosc + pol + 1));

		for (int i = 0; i < dlugosc; i++)
		{
			RodzicIdziecko1[i] = argv[argument][i];
			RodzicIdziecko2[i] = argv[argument][i];
		}

		strcat(RodzicIdziecko1, " ");
		strcat(RodzicIdziecko1, argvDziecka1);

		strcat(RodzicIdziecko2, " ");
		strcat(RodzicIdziecko2, argvDziecka2);
		//printf("argumenty rodzica i dziecka1 po strcat: %s\n", RodzicIdziecko1);



		//***********************************************************************wywolywanie podprocesow
//		printf("Przed fork, PID rodzica = %d\n", (int)getpid());
//		printf("Przed fork, identyfikator grupy rodzica = %d\n", (int)getpgid(0));

		pid_t pidDziecka1 = fork();

		if (pidDziecka1 == 0)      //jesli to dziecko1
		{
//			printf("Przed setpgid identyfikator grupy dziecka = %d\n", (int)getpgid(0));
			setpgid (0, 0);		//utworzenie nowej grupy procesow, w ktorej liderem jest dziecko1 - aktualny proces
//			printf("Po setpgid identyfikator grupy dziecka = %d\n", (int)getpgid(0));
//		   printf("Po fork, PID dziecka = %d\n", (int)getpid());
		   execl("lab4", "lab4", argvDziecka1, RodzicIdziecko1, NULL);
		}

		else if (pidDziecka1 > 0)              //jesli to rodzic
		{
            free(argvDziecka1);
			free(RodzicIdziecko1);

			pid_t pidDziecka2 = fork();

			if (pidDziecka2 == 0)                      //jesli to dziecko2
			{
				setpgid (0, 0);		//utworzenie nowej grupy procesow, w ktorej liderem jest dziecko2 - aktualny proces
				execl("lab4", "lab4", argvDziecka2, RodzicIdziecko2, NULL);
			}
			free(argvDziecka2);
			free(RodzicIdziecko2);



			petla = 1;
			while (petla)	//wieczna petla
			{
				;	//printf("pid w petli: %d\n", (int)getpid());
			}

			if (pidDziecka1 > 0) kill(pidDziecka1, SIGINT);
			//printf("po wyslaniu kill do dziecka1, pid dziecka1: %d\n", pidDziecka1);
			//printf("pid rodzica: %d\n", (int)getpid());
			if (pidDziecka2 > 0) kill(pidDziecka2, SIGINT);

			wait(NULL);
			wait(NULL);
			printf("%d ", (int)getpid());

			if (argc == 2)
				printf("%s\n", argv[1]);
			else
				printf("%s\n", argv[2]);

			sigprocmask(SIG_UNBLOCK, &iset2, NULL);	//odblokowanie sygnalu ctrl+z (SIGTSTP)

			exit(exitStatus);
		}

	}
	if (dlugosc <= 1)
	{
		petla = 1;
		while (petla)	//wieczna petla
		{
			;	//printf("ostatnie dziecko w wiecznej petli, pid: %d\n", (int)getpid());
		}

	   printf ("%d ", (int)getpid());
	   printf ("%s\n", argv[2]);
	   exit(exitStatus);
	}
    return(0);
}