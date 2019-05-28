#include <LCD5110_Basic.h>

#define  SERIAL_SPEED       9600    //velocidade de comunicação da porta serial  
#define  PIN_NVL_AGUA       A4      //número do pino de leitura do nível de água do reservatório - analógico  
#define  PIN_UMD_A          A5      //número do pino de leitura da umidade - analógico  
#define  PIN_BOMBA          2       //número do pino da bomba d'agua  
#define  PIN_LED_B          5       //número do pino do led azul  
#define  PIN_LED_R          6       //número do pino do led vermelho  
#define  PIN_LED_G          7       //número do pino do led verde  
#define  TEMPO_LOOP         5000    //valor padrao para o tempo do loop do programa  
#define  NIVEL_UMID_MIN     700     //valor mínimo para verificação do nível de umidade  
#define  NIVEL_UMID_MED     350     //valor médio para verificação do nível de umidade  
#define  NIVEL_AGUA_MIN     250     //valor mínimo para verificação do nível de água no reservatório  
#define  NIVEL_AGUA_MED     600     //valor médio para verificação do nível de água no reservatório  

#define  LCD_CLK            8       //Pino 8  - Display 5110 - CLK  
#define  LCD_DIN            9       //Pino 9  - Display 5110 - DIN  
#define  LCD_DC             10      //Pino 10 - Display 5110 - DC  
#define  LCD_CE             11      //Pino 11 - Display 5110 - CE  
#define  LCD_RST            12      //Pino 12 - Display 5110 - RST  
#define  LCD_SHOW_DELAY     3000    //delay tempo de exibição do texto no display 5110  

String _textoNivelUmidade   = "";   //guarda o texto com nivel da umidade
String _textoNivelAgua      = "";   //guarda o texto com o nivel de agua
int _nivelUmidade           = 0;    //guarda o valor da umidade lida no pino analógico
int _nivelAgua              = 0;    //guarda o valor do nivel de agua lido no pino analógico
int _percentualUmidade      = 0;    //guarda o percentual da umidade calculada
int _percentualNivelAgua    = 0;    //guarda o percentual do nivel de água calculado

extern uint8_t SmallFont[];        //definição do tamanho da fonte a ser exibida no display
LCD5110 lcd(LCD_CLK, LCD_DIN, LCD_DC, LCD_RST, LCD_CE); //objeto de utilização para o display

//define configurações iniciais
void setup()
{
  //define a velocidade de comunicação da portal serial
  Serial.begin(SERIAL_SPEED);

  //configura o pino da bomba de água para saída
  pinMode(PIN_BOMBA, OUTPUT);
  //configura o pino do led rgb-R
  pinMode(PIN_LED_R, OUTPUT);
  //configura o pino do led rgb-B
  pinMode(PIN_LED_B, OUTPUT);
  //configura o pino do led rgb-G
  pinMode(PIN_LED_G, OUTPUT);
  //configura o pino do sensor de nível de água
  pinMode(PIN_NVL_AGUA, INPUT);
  //configura o pino do sensor de umidade
  pinMode(PIN_UMD_A, INPUT);

  //Inicializando o display
  lcd.InitLCD();
}

//método de loop da aplicação
void loop()
{
  //obtem a umidade da terra na planta
  obtemNivelUmidade();

  //verifica se tem agua no reservatorio
  temAgua();

  //verifica se deve regar
  if (deveRegar())
  {
    //obtem o nível de água no reservatório
    obtemNivelAgua();

    //exibe os níveis no display lcd
    imprimirNiveis();

    //verifica se tem água suficiente para fazer a rega
    if (temAgua())
    {
      //realiza a rega automática
      regarPlanta();
    }
    else
    {
      //avisa que é necessário encher o reservatório de água
      avisoReservatorioVazio();
    }
  }

  //aguarda o tempo definido para realizar nova verificação
  delay(TEMPO_LOOP);
}

//Obtem o valor da umidade no sensor
void obtemNivelUmidade()
{
  Serial.println(" >> obtemNivelUmidade()");

  //liga o led verde sinalizando leitura da umidade
  //digitalWrite(PIN_LED_G,HIGH);
  ligarLed('g');

  // Leitura dos dados analógicos vindos do sensor de umidade de solo
  _nivelUmidade = analogRead(PIN_UMD_A);
  delay(250);

  //faz a relação da leitura com percentual de 0% a 100%
  _percentualUmidade = obtemRelacaoPercentual('u', _nivelUmidade);

  //imprime na serial o valor lido do sensor e o percentual calculado
  Serial.print("Umidade: ");
  Serial.print(_nivelUmidade);
  Serial.print(" = ");
  Serial.print(_percentualUmidade);
  Serial.println("%");

  //desliga led sinalizando fim da leitura
  //digitalWrite(PIN_LED_G,LOW);
  apagarLeds();
}

//Verifica o valor da umidade e retorna se deve ou não regar
bool deveRegar()
{
  Serial.println(" >> deveRegar()");

  bool deveRegar = false;

  if (_nivelUmidade >= NIVEL_UMID_MIN)
  {
    //se o valor aquisitado é >= ao nível mínimo, define que deve regar
    deveRegar = true;
    _textoNivelUmidade = "Umidade Baixa";
  }
  else if (_nivelUmidade < NIVEL_UMID_MIN && _nivelUmidade > NIVEL_UMID_MED)
  {
    //se o valor aquisitado está entre o nível mínimo e médio, não é necessário regar
    _textoNivelUmidade = "Umidade Media";
  }
  else if (_nivelUmidade >= NIVEL_UMID_MED)
  {
    //se o valor aquisitado é maior que o nível médio, não é necessário regar
    _textoNivelUmidade = "Umidade Alta";
  }

  return deveRegar;
}

//faz a leitura do sensor de nível de água
void obtemNivelAgua()
{
  Serial.println(" >> obtemNivelAgua()");

  //liga o led verde sinalizando leitura do nível de água
  //digitalWrite(PIN_LED_G, HIGH);
  ligarLed('g');

  // Leitura dos dados analógicos vindos do sensor de nível de água
  _nivelAgua = analogRead(PIN_NVL_AGUA);
  delay(250);

  //faz a relação da leitura com percentual de 0% a 100%
  _percentualNivelAgua = obtemRelacaoPercentual('a', _nivelAgua);

  //imprime na serial o valor lido do sensor e o percentual calculado
  Serial.print("obtemNivelAgua(): ");
  Serial.print(_nivelAgua);
  Serial.print(" = ");
  Serial.print(_percentualNivelAgua);
  Serial.println("%");

  //desliga led sinalizando fim da leitura
  //digitalWrite(PIN_LED_G, LOW);
  apagarLeds();
}

//obtem o texto do nível de água no reservatório e retorna se tem água
bool temAgua()
{
  Serial.println(" >> temAgua()");

  bool temAgua = true;

  if (_nivelAgua <= NIVEL_AGUA_MIN)
  {
    temAgua = false;
    _textoNivelAgua  = "Nivel Baixo";
  }
  else if (_nivelAgua > NIVEL_AGUA_MIN && _nivelAgua < NIVEL_AGUA_MED)
  {
    _textoNivelAgua = "Nivel Medio";
  }
  else if (_nivelAgua >= NIVEL_AGUA_MED)
  {
    _textoNivelAgua = "Nivel Alto";
  }

  return temAgua;
}

//
void regarPlanta()
{
  Serial.println(" >> regarPlanta()");

  ligarBombaDagua();
  for (int i = 0; i < 10; i++)
  {
    ligarLed('b');
    delay(1000);
    apagarLeds();
    delay(1000);
  }
  desligarBombaDagua();
}

//Liga a bomba d'água
void ligarBombaDagua()
{
  Serial.println(" >> ligarBombaDagua()");
  digitalWrite(PIN_BOMBA, HIGH);
}

//Desliga a bomba d'água
void desligarBombaDagua()
{
  Serial.println(" >> desligarBombaDagua()");
  digitalWrite(PIN_BOMBA, LOW);
}

//emite aviso de reservatório deve ser abastecido
void avisoReservatorioVazio()
{
  Serial.println(" >> avisoReservatorioVazio()");

  //imprime aviso no display
  lcd.clrScr();
  lcd.setFont(SmallFont);
  lcd.print("RESERVATORIO", CENTER, 10);
  lcd.print("VAZIO", CENTER, 30);

  //sinaliza aviso com led vermelho
  do {
    Serial.println("  Reservatorio de agua vazio!!");
    ligarLed('r');
    delay(200);
    apagarLeds();
    delay(200);
    obtemNivelAgua();
    delay(50);

  } while (_nivelAgua < NIVEL_AGUA_MIN);

  lcd.clrScr();
}

//
void imprimirNiveis()
{
  Serial.println(" >> imprimirNiveis()");

  lcd.clrScr();
  lcd.setFont(SmallFont);

  lcd.print(_textoNivelUmidade, 0, 0);
  //lcd.printNumI(_nivelUmidade, 0, 10);
  lcd.printNumI(_percentualUmidade, 60, 10);
  lcd.print("%", RIGHT, 10);

  lcd.print("Reservatorio:", LEFT, 24);
  lcd.print(_textoNivelAgua, RIGHT, 32);
  //lcd.printNumI(_nivelAgua, LEFT, 40);
  lcd.printNumI(_percentualNivelAgua, 60, 40);
  lcd.print("%", RIGHT, 40);

  delay(LCD_SHOW_DELAY);
}

//Obtem do valor recebido a relação percentual de 0% a 100%.
//https://blog.usinainfo.com.br/sensor-de-umidade-de-solo/
int obtemRelacaoPercentual(char tipo, int leitura)
{
  Serial.print(" >> obtemRelacaoPercentual(");
  Serial.print(tipo);
  Serial.print(", ");
  Serial.print(leitura);
  Serial.println(")");

  int relacao = 0;

  if (tipo == 'u') {
    //calcular relação para umidade
    //valor maior significa maior resistência, portanto, menor umidade
    relacao = map(leitura, 1023, 0, 0, 100);
  }
  else {
    //calcular relação para nível de água
    //valor maior representa a quantidade de água no reservatório
    relacao = map(leitura, 1023, 0, 100, 0);
  }

  return relacao;
}

//liga led RGB
//  r = liga led vermelho
//  g = liga led verde
//  b = liga led azul
//  a = liga todos os leds
//  outro valor = apaga todos os leds
void ligarLed(char led)
{
  Serial.print(" >> ligarLed(");
  Serial.print(led);
  Serial.println(")");

  switch (led) {
    case 'r': //liga o led vermelho
      digitalWrite(PIN_LED_R, HIGH);
      break;
    case 'g': //liga o led verde
      digitalWrite(PIN_LED_G, HIGH);
      break;
    case 'b': //liga o led azul
      digitalWrite(PIN_LED_B, HIGH);
      break;
    case 'a': //liga todos os leds
      digitalWrite(PIN_LED_R, HIGH);
      digitalWrite(PIN_LED_R, HIGH);
      digitalWrite(PIN_LED_R, HIGH);
      break;
    default: //apaga todos os leds
      apagarLeds();
      break;
  }
}


//Apaga todos os leds RGB
void apagarLeds()
{
  Serial.println(" >> apagarLeds()");
  digitalWrite(PIN_LED_R, LOW);
  digitalWrite(PIN_LED_G, LOW);
  digitalWrite(PIN_LED_B, LOW);
}
