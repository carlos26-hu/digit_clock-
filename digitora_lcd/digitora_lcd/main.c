/*
 * digitora_lcd.c
 *
 * Created: 2019. 06. 22. 21:50:59
 * Author : Lovasz Karoly
 */ 

/********************* MACRO DEFINITIONS AND INCLUDES **************/
#define F_CPU 16000000UL

#define USART0_BAUDRATE 9600
#define UBRR_ERTEK ((F_CPU / (USART0_BAUDRATE * 16UL)) - 1)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/lcd.h>

/********************* DECLARATION OF FUNCTIONS *************************/

char BillMatrixRead(unsigned char Row);
void HW_Init(void);           //az inicializ?l? f?ggv?ny protot?pusa
void SendCharOnUart0(char data);
void SendStringOnUart0(char *ip);
void anker(void);
void orakijelzes(void);
void orabeallitas(void);

/********************* DECLARATION OF GLOBAL VARIABLES *************************/
volatile int Counter = 0,ora=0,perc=0,mp=0,i=0, ertek=0, beall=0;
volatile char UtoljaraLenyomottGomb='0', temp='N', felenged = 1;
int main(void)
{
	DDRC = 0b11111000;
	/// DDRA = 0xff;
	DDRF = 0b00001110;//  E,RW,RS output
	DDRE = 0b11110000;//  DB7,DB6,DB5,DB4 output
	
	HW_Init();
	init_lcd();       //lcd init
	
	///	 putch('a');  //betu kiiratas

	StringToLCD("Ora beallitas:k0");  //string kiiratas
	command(S_LINE_BEG);   //kurzor to 2. sor eleje
	StringToLCD("Ora inditas:k1");
	command(T_LINE_BEG);
	StringToLCD("Ora leallitas:k2");		//	 StringToLCD("masodik sor");  //string kiiratas
	_delay_ms(1000);
	command(CLR_SCR);
	orabeallitas();
	
	TIMSK=1;
	sei();
	
	
	///	char UtoljaraLenyomottGomb='0', temp='N', felenged = 1;
	while (1)
	{
		switch(PING)
		{
			case 2:beall=2;break;
			case 4:beall=4;break;
			default:break;	
		}
		
		if (beall ==2) TCCR0=7;
		if (beall ==4) TCCR0=0;
		

		
		
		temp = BillMatrixRead(1); 	//temp = 1         feleng: temp == 'N'
		if(temp!='N'){ UtoljaraLenyomottGomb=temp;felenged = 0;continue;} //utgomb=1
		if(felenged == 0) {felenged = 1; orabeallitas();}
		
		temp = BillMatrixRead(2);
		if(temp!='N'){ UtoljaraLenyomottGomb=temp;felenged = 2;continue;}
		if(felenged == 2) {felenged = 3; orabeallitas();}
		
		temp = BillMatrixRead(3);
		if(temp!='N'){ UtoljaraLenyomottGomb=temp;felenged = 4;continue;}
		if(felenged == 4) {felenged = 5; orabeallitas();}
		
		temp = BillMatrixRead(4);
		if(temp!='N'){ UtoljaraLenyomottGomb=temp;felenged = 6;continue;}
		if(felenged == 6) {felenged = 7; orabeallitas();}
			
	
	}
}

char BillMatrixRead(unsigned char Row) //param: 1 elso sor, 2 masodik sor stb.
{
	//adott sor feszultseg ala helyezese kezeles
	switch(Row)
	{
		case 1: PORTC = 0b00001000; break; //1.sor fesz ala
		case 2: PORTC = 0b00010000; break; //2.sor fesz ala
		case 3: PORTC = 0b00100000; break; //3.sor fesz ala
		case 4: PORTC = 0b01000000; break; //4.sor fesz ala
		default:break;
	}
	_delay_ms(1);
	
	if( (~PINC)&1) //ha 1,4,7,* gombok kozul le van nyomva (1.oszlop)
	{
		switch (Row)
		{
			case 1: return '1'; break;
			case 2: return '4'; break;
			case 3: return '7'; break;
			case 4: return '*'; break;
			default: break;
		}
	}
	else if( (~PINC)&2 )//ha 2,5,8,0 gombok kozul le van nyomva (2.oszlop)
	{
		switch (Row)
		{
			case 1: return '2'; break;
			case 2: return '5'; break;
			case 3: return '8'; break;
			case 4: return '0'; break;
			default: break;
		}
	}
	else if( (~PINC)&4)//ha 3,6,9,# gombok kozul le van nyomva (3.oszlop)
	{
		switch (Row)
		{
			case 1: return '3'; break;
			case 2: return '6'; break;
			case 3: return '9'; break;
			case 4: return '#'; break;
			default: break;
		}
	}
	return 'N';  //'N' betu, ha nincs lenyomva gomb
}

/****************** DEFINITIONS OF SERVICES *******************/
void HW_Init(void)
{
	cli(); 		//glob?lis megszak?t?sok kikapcsol?sa


	//Nyom?gombsor inicializ?l?sa
	DDRG = 0;       //kapcsol?sor portj?t bemenetk?nt inicializ?ljuk
	PORTG = 0;      //felh?z?ellen?ll?sok kikapcsol?sa

	
	//TIMER0 inicializ?l?sa
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



	//SLEEP MODE El?rhet? legyen
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
		mp=0;		//mp-t null?zzuk
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
	command(CLR_SCR);
	command(S_LINE_BEG);
	
	putch(ora/10+48);		// ora tizes helyiertekenek elkuldese
	putch(ora%10+48);		//ora egyes helyiertekenek elkuldese
	putch(':');
	putch(perc/10+48);	// perc tizes helyiertekenek elkuldese
	putch(perc%10+48);	//perc egyes helyiertekenek elkuldese
	putch(':');
	putch(mp/10+48);		// mp tizes helyiertekenek elkuldese
	putch(mp%10+48);		//mp egyes helyiertekenek elkuldese
}

void orabeallitas(void )
{
	
	
	
	TCCR0 =0;						// az ora mukodeset leallitjuk
	switch(UtoljaraLenyomottGomb)			//fogadott adat ertekelese
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
			command(HOME);
			StringToLCD("Hibas bevitel, ird be a pontos orat!");	// tajekozatas
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
			command(HOME);
			StringToLCD("Hibas bevitel, ");	//tajekoztatas
			command(S_LINE_BEG);
			StringToLCD("ird be a pontos");
			command(T_LINE_BEG);
			StringToLCD("percet");
		}else
		{
			i=0;					//segedvaltozo nullazas
			beall=0;				//uzemmod visszaallitva mukodesre
			command(HOME);
			StringToLCD("Ora beallitva, ");
			command(S_LINE_BEG);
			StringToLCD("inditas k1");
		}
		
	}
	
	orakijelzes();						//ido kijelzese
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
