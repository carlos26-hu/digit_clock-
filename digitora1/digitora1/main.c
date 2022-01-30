/*
 * digitora1.c
 *
 * Created: 2019. 06. 21. 10:32:02
 * Author : Lovasz Karoly 
 */ 


///TESZT OK!!!!

/********************* MACRO DEFINITIONS AND INCLUDES **************/
#define F_CPU 16000000UL       //CPU órajele 16MHz!  _delay_ms(); fgv-hez szükséges, fontos, hogy
//az include elé kerüljön, különben hibás mûködés!!!

#define USART_BAUDRATE 9600  // soros kommunikacio sebessege: 9600 bps
#define UBRR_ERTEK ((F_CPU / (USART_BAUDRATE * 16UL)) - 1) // UBRR

#include <avr/io.h>            //Utasításkészlet importálása
#include <avr/interrupt.h>     //Megszakításkezelõ rutinok importálása
#include <util/delay.h>




/********************* DECLARATION OF FUNCTIONS *************************/
void HW_Init(void);           //az inicializáló függvény prototípusa
void SendCharOnUart0(char data);
void SendStringOnUart0(char *ip);

void anker(void);
void orakijelzes(void);
void orabeallitas(void);
/********************* DECLARATION OF GLOBAL VARIABLES *************************/
volatile int Counter = 0,ora=0,perc=0,mp=0,i=0, ertek=0, beall=0;

/********************* FUNCTION MAIN *******************************/
int main(void)
{
	HW_Init();
	SendCharOnUart0('\f'); //soros vonalon új oldal nyitása
	SendStringOnUart0("UART OK!\n\n\r");
	SendStringOnUart0("Ora beallitasa:o\n\r");
	SendStringOnUart0("Ora inditasa:i\n\r");
	SendStringOnUart0("Ora leallitasa:s\n\r");
	SendStringOnUart0("\n\n\r");
	
	sei();			//global interrupts enable
	TIMSK=1;		//Timer0 Interrupt enable
	TCCR0 = 0;		//Timer0 off

	while (1)
	{
		
	}
	
	return 0;
}
//end of function main



/****************** DEFINITIONS OF SERVICES *******************/
void HW_Init(void)
{
	cli(); 		//globális megszakítások kikapcsolása


	//Nyomógombsor inicializálása
	DDRG = 0;       //kapcsolósor portját bemenetként inicializáljuk
	PORTG = 0;      //felhúzóellenállások kikapcsolása

	
	//TIMER0 inicializálása
	//	TCCR0=0b00000111;	//Timer0 inic:16Mhz/256/1024 = 61,0351Hz
	//	TIMSK=1;  			//Timer0 interrupt enable



	//USART0 soros komm. init
	// 9600 bps soros kommunikacio sebesseg beallitasa
	UBRR0L = UBRR_ERTEK;    // UBRR_ERTEK also 8 bitjenek betoltese az UBRRL regiszterbe
	UBRR0H = (UBRR_ERTEK>>8);   // UBRR_ERTEK felso 8 bitjenek betoltese az UBRRH regiszterbe

	// Aszinkron mod, 8 Adat Bit, Nincs Paritas Bit, 1 Stop Bit
	UCSR0C |= (0 <<USBS ) | (3 << UCSZ0);

	//Ado es Vevo aramkorok bekapcsolasa + az RX interrupt engedelyezese
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);



	//SLEEP MODE Elérhetõ legyen
	MCUCR = 0b00100000;  //sleep mode enable

}



void SendCharOnUart0(char data) // Ez a fuggveny a kuldendo adatot beirja az UDR regiszter kimeno pufferjebe
{
	while(!(UCSR0A & (1<<UDRE0)))  // Varakozas amig az Ado kesz nem lesz az adatkuldesre
	{
		//Varakozas
	}
	// Az Ado mar kesz az adatkuldesre, a kuldendo adatot a kimeno pufferjebe irjuk
	UDR0=data;
}


void SendStringOnUart0(char *ip)
{
	while(*ip)
	{
		SendCharOnUart0(*ip);
		ip++;
	}
}


void anker(void)
{	
	mp++;			//mp novelese
	if(mp==60)		// ha mp eleri a 60-at
	{
		perc++;		//percet noveljuk
		mp=0;		//mp-t nullázzuk
	}
	if(perc==60)	//ha perc eleri a 60-at
	{
		ora++;		//orat noveljuk
		perc=0;		//percet nullazzuk
	}
	if(ora==24)		//ha ora eleri a 24-et
	{
		ora=0;		//orat nullazzuk
	}
}

void orakijelzes(void)
{
	SendStringOnUart0("\r");
	SendCharOnUart0(ora/10+48);		// ora tizes helyiertekenek elkuldese
	SendCharOnUart0(ora%10+48);		//ora egyes helyiertekenek elkuldese
	SendCharOnUart0(':');      
	SendCharOnUart0(perc/10+48);	// perc tizes helyiertekenek elkuldese
	SendCharOnUart0(perc%10+48);	//perc egyes helyiertekenek elkuldese
	SendCharOnUart0(':');      
	SendCharOnUart0(mp/10+48);		// mp tizes helyiertekenek elkuldese
	SendCharOnUart0(mp%10+48);		//mp egyes helyiertekenek elkuldese
}
/******************** INTERRUPT SERVICE ROUTINES *****************************/
ISR(TIMER0_OVF_vect)
{
	Counter++;
	if(Counter>=61) //ha eltelt 1 mp
	{
		Counter = 0;
		
		anker();
		orakijelzes();

	}
	

}

ISR(USART0_RX_vect)  //soros adat érkezett, interrrupt, ide adódik a vezérlés
{
	char ReceivedByte0 = UDR0; //erkezett adat tarolasa
	
	
	if (beall == 0) // mukodesi uzem
	{
		
		switch(ReceivedByte0) //fogadott adat ertekelese
		{
		case 'i': TCCR0=7; break;		//Timer0 inditas
		case 's': TCCR0=0; break;		//Timer0 leallitas
		case 'o': beall=1; break;		// beallito uzemet bekapcsoljuk
		default:break;
		}
	}else 
	if (beall ==1)						//beallito uzem
	{	
		TCCR0 =0;						// az ora mukodeset leallitjuk
		switch(ReceivedByte0)			//fogadott adat ertekelese
		{
			case '0': ertek=0; break;	//a fogadott ascii karakter helyett 0 int tipust irunk az ertek valtozoba
			case '1': ertek=1; break;	//a fogadott ascii karakter helyett 1 int tipust irunk az ertek valtozoba
			case '2': ertek=2; break;	//a fogadott ascii karakter helyett 2 int tipust irunk az ertek valtozoba
			case '3': ertek=3; break;	//a fogadott ascii karakter helyett 3 int tipust irunk az ertek valtozoba
			case '4': ertek=4; break;	//a fogadott ascii karakter helyett 4 int tipust irunk az ertek valtozoba
			case '5': ertek=5; break;	//a fogadott ascii karakter helyett 5 int tipust irunk az ertek valtozoba
			case '6': ertek=6; break;	//a fogadott ascii karakter helyett 6 int tipust irunk az ertek valtozoba
			case '7': ertek=7; break;	//a fogadott ascii karakter helyett 7 int tipust irunk az ertek valtozoba
			case '8': ertek=8; break;	//a fogadott ascii karakter helyett 8 int tipust irunk az ertek valtozoba
			case '9': ertek=9; break;	//a fogadott ascii karakter helyett 9 int tipust irunk az ertek valtozoba
			default:break;
		}
		if(i==0)						// ora tizes helyiertekenek bekerese (segedvaltozo,hogy melyik helyierteket irjuk)
		{			
			ora = ertek*10;				//ora tizedes helyiertekenek megadasa
			i++;						//segedvaltozo novelese
			orakijelzes();				//ido kijelzese
		}else
		if (i== 1)						// ora egyes helyiertekenek bekerese
		{
			ora = ora+ertek;			//ora egyes helyierteket hozzaadjuk
			i++;						//segedvaltozo novelese
			orakijelzes();				//ido kijelzese
			if ( ora > 23)				// hibakezeles, ha ora nagyobb mint 23 
			{
				i=0;					// segedvaltozo nullazasa, hogy ujra bekerje az ora erteket	
				SendStringOnUart0("\n\rHibas bevitel, ird be a pontos orat!\n\r");	// tajekozatas
			}
		}else
		if (i ==2)						// perc tizes helyiertekenek bekerese
		{
			perc = ertek*10;			// perc tizes helyiertekenek megadasa
			i++;						//segedvaltozo novelese
			orakijelzes();				//ido kijelzese
		}else
 		if (i==3)						// perc egyes helyiertekenek bekerese
		{
			perc = perc+ertek;			// perc egyes helyiertekenek megadasa
			if (perc > 59)				//hibakezeles, ha perc nagyobb mint 59
			{
			i=2;						// segedvaltozo 2-es ertek, hogy ujra bekerje a percet
			SendStringOnUart0("\n\rHibas bevitel, ird be a pontos percet!\n\r");	//tajekoztatas
			}else 
			{
				i=0;					//segedvaltozo nullazas
				beall=0;				//uzemmod visszaallitva mukodesre
				SendStringOnUart0("\n\rOra beallitva, inditas i gombbal\n\r");
			}
			
		}
	}
	orakijelzes();						//ido kijelzese
	
}
