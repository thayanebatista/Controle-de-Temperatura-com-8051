#include<reg51.h>
#include<stdio.h>
// definição de portas 
sbit oe=P2^3; 
sbit ale=P2^4;  
sbit start=P2^5; 
sbit eoc=P2^6;  
sbit clk=P2^7;
#define lcdport P0  //DEFINE A PORTA P0 COMO OS DADOS DO LCD 
sbit rs=P2^0;
sbit rw=P2^1;
sbit en=P2^2;
sbit rele=P3^2;
#define input_port P1  //ADC
int result[5],number, i;
char rele_status[11] = "desativado";

//--------------------------------------------------------
// lookup table de conversão de valores de temperatura a partir do valor lido pelo adc em hexa
code float temp[] = { 
	0.00, 0.59, 1.18, 1.76, 2.35, 2.94, 3.53, 4.12, 4.71, 5.29, 5.88, 6.47, 7.06, 7.65, 8.24, 8.82, 9.41, 10.00, 10.59, 11.18, 11.76, 12.35, 12.94, 13.53, 
	14.12, 14.71, 15.29, 15.88, 16.47, 17.06, 17.65, 18.24, 18.82, 19.41, 20.00, 20.59, 21.18, 21.76, 22.35, 22.94, 23.53, 24.12, 24.71, 25.29, 25.88, 26.47, 
	27.06, 27.65, 28.24, 28.82, 29.41, 30.00, 30.59, 31.18, 31.76, 32.35, 32.94, 33.53, 34.12, 34.71, 35.29, 35.88, 36.47, 37.06, 37.65, 38.24, 38.82, 39.41, 
	40.00, 40.59, 41.18, 41.76, 42.35, 42.94, 43.53, 44.12, 44.71, 45.29, 45.88, 46.47, 47.06, 47.65, 48.24, 48.82, 49.41, 50.00, 50.59, 51.18, 51.76, 52.35, 
	52.94, 53.53, 54.12, 54.71, 55.29, 55.88, 56.47, 57.06, 57.65, 58.24, 58.82, 59.41, 60.00, 60.59, 61.18, 61.76, 62.35, 62.94, 63.53, 64.12, 64.71, 65.29, 
	65.88, 66.47, 67.06, 67.65, 68.24, 68.82, 69.41, 70.00, 70.59, 71.18, 71.76, 72.35, 72.94, 73.53, 74.12, 74.71, 75.29, 75.88, 76.47, 77.06, 77.65, 78.24, 
	78.82, 79.41, 80.00, 80.59, 81.18, 81.76, 82.35, 82.94, 83.53, 84.12, 84.71, 85.29, 85.88, 86.47, 87.06, 87.65, 88.24, 88.82, 89.41, 90.00, 90.59, 91.18, 
	91.76, 92.35, 92.94, 93.53, 94.12, 94.71, 95.29, 95.88, 96.47, 97.06, 97.65, 98.24, 98.82, 99.41, 100.00, 100.59, 101.18, 101.76, 102.35, 102.94, 103.53, 
	104.12, 104.71, 105.29, 105.88, 106.47, 107.06, 107.65, 108.24, 108.82, 109.41, 110.00, 110.59, 111.18, 111.76, 112.35, 112.94, 113.53, 114.12, 114.71, 
	115.29, 115.88, 116.47, 117.06, 117.65, 118.24, 118.82, 119.41, 120.00, 120.59, 121.18, 121.76, 122.35, 122.94, 123.53, 124.12, 124.71, 125.29, 125.88, 
	126.47, 127.06, 127.65, 128.24, 128.82, 129.41, 130.00, 130.59, 131.18, 131.76, 132.35, 132.94, 133.53, 134.12, 134.71, 135.29, 135.88, 136.47, 137.06, 
	137.65, 138.24, 138.82, 139.41, 140.00, 140.59, 141.18, 141.76, 142.35, 142.94, 143.53, 144.12, 144.71, 145.29, 145.88, 146.47, 147.06, 147.65, 148.24, 
	148.82, 149.41, 150.00
};

//--------------------------------------------------------
// interrupção que gera o clock para o conversor adc
void timer0() interrupt 1 { 
	clk = ~clk;
  TF0 = 0; 
}
//--------------------------------------------------------
//função que cria delay de 1ms
void delay(int count) {
	int i,j;
    for(i=0;i<count;i++)    
      for(j=0;j<1275;j++); 
}

//--------------------------------------------------------
// função escreve no lcd
void write_lcd(int ch) { 
  lcdport=ch;
  rs=1;
  rw=0;
  en=1;
  delay(1);
  en=0;
}
//--------------------------------------------------------
// envia comando para habilitar o lcd
void command_lcd(char ch) { 
  lcdport=ch;
  rs=0;
  rw=0;
  en=1;
  delay(5);
  en=0;
}
//--------------------------------------------------------
// envia uma string para o display lcd
void print_lcd(char *str) {
  while(*str) {
    write_lcd(*str);
		delay(4);
    str++;
  }
}
//--------------------------------------------------------
// Método para enviar uma string para o display lcd e retorna o tamanho da string
void print_lcd_shifting(char *str) { 
	command_lcd(0x01);
  while(*str) {
		command_lcd(0x06);
		command_lcd(0x07);
    write_lcd(*str);
    str++;
  }
}
//--------------------------------------------------------
// envia uma string por display lcd de forma que ele passe 
//de um lado ao outro no display e retorna o tamanho da string enviada
void print_and_get_out(char *str) { 
	print_lcd_shifting(str);
	for(i=0; i <= 32; i++) {
		command_lcd(0x1c);
		delay(5);
	}
	command_lcd(0x01);
}
//--------------------------------------------------------
// fnção para inicializar o LCD
void ini_lcd() { 
  delay(20);
  command_lcd(0x38); // seta o display para 2 linhas
  delay(10);
  command_lcd(0x0c); // oculta o cursor no display
  delay(10);
  command_lcd(0x01); // limpa do display
  delay(10);
  command_lcd(0x80); //coloca o cursor no inicio da tela
  delay(10);
}
//--------------------------------------------------------
// converte os dados lidos pelo conversor adc em hexadecimal para 
//o valor de temperatura equivalente e envia para o display lcd 
//juntamente com o estado do rele
void show_lcd() { 
  sprintf(result,"%0.2f",temp[number]);//tempo de 2s de exibição de boas vindas
  print_lcd(result);
	print_lcd(" ");
	if (rele == 1) {
		print_lcd("    DESLIGADO");
	} else {
		print_lcd("    LIGADO   ");
	}
}
//--------------------------------------------------------
// função para ler o dados obtidos pelo conversor adc
void read_adc() { 
  ale=1;
  start=1;
  delay(5);
  ale=0;
  start=0;
  while(eoc==1);
  while(eoc==0);
  oe=1;
  number=input_port;
  delay(5);
  oe=0;
}
//--------------------------------------------------------
// faz o envio de um caracter via serial
void serial_data(char ch) { 
	SBUF = ch; 
	while(TI == 0); // espera o fim do envio do caractere
	TI = 0; // libera o canal de serial
}
//--------------------------------------------------------
// envia caractere por caractere de uma string via serial

void print_serial(char *str) { 
	while(*str) {
    serial_data(*str);
    str++;
  }
}
//--------------------------------------------------------
// método que envia os dados para serial
void show_serial() { 
	serial_data(0x0d); // envia uma quebra de linha
  print_serial(result); // envia o resultado da conversão de temperatura
	if (rele == 1) { // envia o estado do rele
		print_serial(" Desativado");
	} else {
		print_serial(" Ativado");
	}
	serial_data(0x0d); // envia uma quebra de linha
}

void main() {
  // configuração do adc
  eoc=1;
  ale=0;
  oe=0;
  start=0;

  // configuração serial
  IE = 0x82;
  SCON = 0x50;

  // configuração do timer
  TMOD = 0x22;
  TH0 = 0x9b;
  TH1 = 0xfd;
  TL0 = 0x9b;
  TR1 = 1;
  TR0 = 1;
	
  ini_lcd(); // chama o método de inicialização do display
  print_lcd("Bem Vindo");
	delay(200);
  command_lcd(0x01); // limpa o display
	print_and_get_out("Bem Vindo");
	print_and_get_out("Bem Vindo");
	print_and_get_out("Bem Vindo");
  command_lcd(0x01); // limpa o display
  command_lcd(0x06); // para de "rodar" o display
  print_lcd("Temp:     Rele:");
  while (1) { // entra em loop inifinito
    command_lcd(0xc0); // coloca o cursor do display na segunda linha
    read_adc(); // le os dados de temperatura fornecidos pelo conversor adc
		
		// verifica a temperatura atual e controla o rele por meio de histerese
		if (number < 39) { 
			rele = 0;
		}
		if (number > 46) {
			rele = 1;
		}
		show_lcd(); // atualiza os dados no display de acordo com as medições
		if (RI == 1) { // se receber algum dado da serial
			RI = 0; // espera o termino do envio do dado
      if (SBUF == 'm') { // se o dado for o caractere 'm'
        show_serial(); // envia as informações de temperatura e do relê via serial
      }
    }
  }
}