#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "lwip/tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Inclusão das suas bibliotecas e do header PIO
#include "ssd1306.h"
#include "font.h"
#include "bmp280.h"
#include "aht20.h"
#include "ws2812.pio.h"

// ===============================================
//         CONFIGURAÇÕES DA REDE WIFI
// ===============================================
#define WIFI_SSID "Nando Barbearia"
#define WIFI_PASSWORD "nando2661"

// ===============================================
//   CORPO DA PÁGINA HTML (COM GRÁFICOS ATUALIZADOS)
// ===============================================

const char HTML_BODY[] =
"<!DOCTYPE html>"
"<html lang=\"pt-br\">"
"<head>"
  "<meta charset=\"UTF-8\">"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
  "<title>Estação Meteorológica | Projeto Técnico</title>"
  "<style>"
    ":root {"
      "--bg-color: #f4f4f0;"
      "--grid-color: #dcdcdc;"
      "--text-color: #1a1a1a;"
      "--text-muted: #555;"
      "--border-color: #888;"
      "--card-bg: #ffffff;"
      "--primary-accent: #0044cc;"
      "--danger-accent: #d93025;"
      "--card-shadow-color: #ccc;"
      "--card-shadow: 4px 4px 0px 0px var(--card-shadow-color);"
      "--card-shadow-hover: 6px 6px 0px 0px var(--card-shadow-color);"
    "}"
    "* { box-sizing: border-box; margin: 0; padding: 0; }"
    "body {"
      "font-family: 'Roboto Mono', 'Courier New', monospace;"
      "background-color: var(--bg-color);"
      "background-image: linear-gradient(var(--grid-color) 1px, transparent 1px), linear-gradient(to right, var(--grid-color) 1px, transparent 1px);"
      "background-size: 20px 20px;"
      "color: var(--text-color);"
      "padding: 20px;"
      "min-height: 100vh;"
      "line-height: 1.6;"
    "}"
    ".container { max-width: 1400px; margin: 0 auto; }"
    ".header { text-align: center; margin-bottom: 40px; background: var(--bg-color); padding: 10px; }"
    ".header h1 { font-size: 2.5rem; font-weight: 700; color: var(--text-color); border-bottom: 3px double var(--text-color); display: inline-block; padding-bottom: 5px; }"
    ".header p { color: var(--text-muted); font-size: 1rem; margin-top: 10px; }"
    ".dashboard { display: grid; grid-template-columns: repeat(auto-fit, minmax(350px, 1fr)); gap: 30px; }"
    ".card { background: var(--card-bg); border-radius: 4px; border: 2px solid #000; padding: 25px; box-shadow: var(--card-shadow); transition: all 0.2s ease-out; }"
    ".card:hover { transform: translate(-2px, -2px); box-shadow: var(--card-shadow-hover); }"
    ".card-header { display: flex; align-items: center; margin-bottom: 20px; padding-bottom: 15px; border-bottom: 2px dashed var(--border-color); }"
    ".card-header h2 { margin: 0; font-size: 1.3rem; font-weight: 700; color: var(--text-color); }"
    ".card-header .icon { font-size: 1.5rem; margin-right: 12px; }"
    ".readings-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 15px; }"
    ".reading-item { text-align: center; padding: 15px; background: none; border-radius: 0; border: 1px dashed var(--border-color); }"
    ".reading-item .value { font-size: 1.7rem; font-weight: 700; color: var(--text-color); }"
    ".alert-indicator { color: var(--danger-accent); font-weight: bold; animation: blink 1s step-start infinite; font-size: 1.2rem; margin-left: 5px; }"
    "@keyframes blink { 50% { opacity: 0; } }"
    ".chat-container { display: flex; flex-direction: column; height: 320px; }"
    ".chat-messages { flex: 1; overflow-y: auto; border: 2px solid #000; background: #f9f9f9; padding: 15px; margin-bottom: 15px; display: flex; flex-direction: column; gap: 12px; }"
    ".chat-input { flex: 1; background: #fff; color: var(--text-color); border: 2px solid #000; padding: 10px; font-family: 'Roboto Mono', monospace; }"
    ".chat-message { padding: 10px; max-width: 85%; line-height: 1.4; font-size: 0.9rem; border: 1px solid #ccc; }"
    ".message-bot { background: #e9e9e9; align-self: flex-start; }"
    ".message-user { background: #dbeaff; align-self: flex-end; border-color: var(--primary-accent); }"
    ".setting-item { display: flex; align-items: center; justify-content: space-between; padding: 10px; border: 1px dashed var(--border-color); margin-bottom: 10px; }"
    ".input-number { border: 2px solid #000; padding: 8px; width: 80px; text-align: center; font-family: 'Roboto Mono', monospace; }"
    ".btn { background: var(--primary-accent); color: white; border: 2px solid #000; padding: 8px 16px; cursor: pointer; font-weight: 700; font-family: 'Roboto Mono', monospace; }"
    ".chart-container { width: 100%; height: 280px; }"
    ".card-full-width { grid-column: 1 / -1; }"
  "</style>"
  "<script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>"
"</head>"
"<body>"
  "<div class=\"container\">"
    "<header class=\"header\">"
      "<h1>[ Estação Meteorológica // Relatório Técnico ]</h1>"
      "<p>Monitoramento de dados atmosféricos em tempo real</p>"
    "</header>"
    "<main class=\"dashboard\">"
      "<div class=\"card\">"
        "<div class=\"card-header\"><span class=\"icon\">🌍</span><h2>Módulo de Sensores</h2></div>"
        "<div class=\"readings-grid\">"
          "<div class=\"reading-item\"><div class=\"label\">TEMP_BMP280</div><div class=\"value\"><span id=\"temp_bmp\">--</span>°C</div></div>"
          "<div class=\"reading-item\"><div class=\"label\">UMIDADE</div><div class=\"value\"><span id=\"umidade\">--</span>%<span id=\"umidadeAlert\" class=\"alert-indicator\" style=\"display:none;\">!</span></div></div>"
          "<div class=\"reading-item\"><div class=\"label\">ALTITUDE</div><div class=\"value\"><span id=\"altitude\">--</span>m<span id=\"altitudeAlert\" class=\"alert-indicator\" style=\"display:none;\">!</span></div></div>"
          "<div class=\"reading-item\"><div class=\"label\">PRESSÃO</div><div class=\"value\"><span id=\"pressao\">--</span>kPa</div></div>"
          "<div class=\"reading-item\"><div class=\"label\">TEMP_AHT20</div><div class=\"value\"><span id=\"temp_aht\">--</span>°C</div></div>"
        "</div>"
      "</div>"
      "<div class=\"card\">"
        "<div class=\"card-header\"><span class=\"icon\">💬</span><h2>Terminal de Logs</h2></div>"
        "<div class=\"chat-container\">"
          "<div class=\"chat-messages\" id=\"chat-messages\"></div>"
          "<div class=\"chat-input-container\">"
            "<input type=\"text\" class=\"chat-input\" id=\"chat-input\" placeholder=\"Comando...\" onkeydown=\"if(event.key==='Enter'){handleChatInput()}\">"
            "<button class=\"btn\" onclick=\"handleChatInput()\">Enviar</button>"
          "</div>"
        "</div>"
      "</div>"
      "<div class=\"card\">"
        "<div class=\"card-header\"><span class=\"icon\">🔧</span><h2>Painel de Calibração</h2></div>"
        "<div class=\"settings-grid\">"
          "<form onsubmit=\"return enviarConfig('configalt', 'limiteAlt')\">"
            "<div class=\"setting-item\"><label>Limite Alerta Altitude (m):</label><input type=\"number\" class=\"input-number\" id=\"limiteAlt\" value=\"800\"><button type=\"submit\" class=\"btn\">Set</button></div>"
          "</form>"
          "<form onsubmit=\"return enviarConfig('configmax', 'limiteMax')\">"
            "<div class=\"setting-item\"><label>Limite Alerta Umidade (%):</label><input type=\"number\" class=\"input-number\" id=\"limiteMax\" value=\"80\"><button type=\"submit\" class=\"btn\">Set</button></div>"
          "</form>"
          "<form onsubmit=\"return enviarConfig('configumi', 'calibraUmi')\">"
            "<div class=\"setting-item\"><label>Offset Umidade (%):</label><input type=\"number\" class=\"input-number\" id=\"calibraUmi\" value=\"0\" step=\"0.1\"><button type=\"submit\" class=\"btn\">Set</button></div>"
          "</form>"
        "</div>"
      "</div>"
      "<div class=\"card card-full-width\"><div class=\"card-header\"><span class=\"icon\">💧</span><h2>Gráfico: Umidade</h2></div><div class=\"chart-container\"><canvas id=\"humidityChart\"></canvas></div></div>"
      "<div class=\"card card-full-width\"><div class=\"card-header\"><span class=\"icon\">🏔️</span><h2>Gráfico: Altitude</h2></div><div class=\"chart-container\"><canvas id=\"altitudeChart\"></canvas></div></div>"
    "</main>"
  "</div>"
  "<script>"
    "let lastValues = {};"
    "let humidityChart, altitudeChart;"
    "let humidityData = [], altitudeData = [];"
    "const MAX_DATA_POINTS = 20;"
    
    "function createChart(ctx, label, color, gridColor) {"
      "const gradient = ctx.createLinearGradient(0, 0, 0, 280);"
      "gradient.addColorStop(0, color + '80');"
      "gradient.addColorStop(1, color + '05');"
      "return new Chart(ctx, {"
        "type: 'line',"
        "data: {"
          "labels: [],"
          "datasets: [{"
            "label: label,"
            "data: [],"
            "borderColor: color,"
            "backgroundColor: gradient,"
            "tension: 0.4,"
            "fill: true,"
            "borderWidth: 2.5,"
            "pointRadius: 0,"
            "pointHoverRadius: 5,"
            "pointBackgroundColor: color,"
            "pointBorderColor: '#fff',"
            "pointHoverBorderWidth: 2"
          "}]"
        "},"
        "options: {"
          "responsive: true, maintainAspectRatio: false,"
          "scales: {"
            "y: {"
              "ticks: { color: '#555', font: { family: \"'Roboto Mono', monospace\" } },"
              "grid: { color: gridColor, borderDash: [5, 5], drawBorder: false }"
            "},"
            "x: {"
              "ticks: { color: '#555', font: { family: \"'Roboto Mono', monospace\" } },"
              "grid: { display: false, drawBorder: false }"
            "}"
          "},"
          "plugins: {"
            "legend: {"
              "position: 'top', align: 'end',"
              "labels: {"
                "color: '#333',"
                "font: { family: \"'Roboto Mono', monospace\", size: 14, weight: 'bold' },"
                "boxWidth: 15, padding: 20, usePointStyle: true"
              "}"
            "},"
            "tooltip: {"
              "enabled: true,"
              "backgroundColor: '#1a1a1a',"
              "titleFont: { family: \"'Roboto Mono', monospace\", size: 14 },"
              "bodyFont: { family: \"'Roboto Mono', monospace\", size: 12 },"
              "padding: 10, cornerRadius: 4, displayColors: false"
            "}"
          "}"
        "}"
      "});"
    "}"
    
    "function updateChart(chart, dataArray, newData) {"
      "dataArray.push(newData);"
      "if (dataArray.length > MAX_DATA_POINTS) dataArray.shift();"
      "chart.data.labels = dataArray.map((_, i) => `${i + 1}`);"
      "chart.data.datasets[0].data = dataArray;"
      "chart.update('none');"
    "}"
    
    "function addChatMessage(message, sender) {"
      "const chatBox = document.getElementById('chat-messages');"
      "const msgDiv = document.createElement('div');"
      "msgDiv.className = `chat-message message-${sender}`;"
      "msgDiv.textContent = message;"
      "chatBox.appendChild(msgDiv);"
      "chatBox.scrollTop = chatBox.scrollHeight;"
    "}"
    
    "function handleChatInput() {"
      "const input = document.getElementById('chat-input');"
      "const query = input.value.toLowerCase().trim();"
      "if (!query) return;"
      "addChatMessage(input.value, 'user');"
      "let response = 'Comando inválido. Tente: umidade, temperatura, pressao, altitude.';"
      "if (query.includes('umidade')) response = `> UMIDADE ATUAL: ${lastValues.humidity?.toFixed(1) ?? '--'}%.`;"
      "if (query.includes('temperatura')) response = `> TEMPERATURAS: BMP ${lastValues.temp_bmp?.toFixed(1) ?? '--'}°C | AHT ${lastValues.temp_aht?.toFixed(1) ?? '--'}°C.`;"
      "if (query.includes('pressao') || query.includes('pressão')) response = `> PRESSÃO ATMOSFÉRICA: ${lastValues.pressure?.toFixed(2) ?? '--'} kPa.`;"
      "if (query.includes('altitude')) response = `> ALTITUDE ESTIMADA: ${lastValues.altitude ?? '--'}m.`;"
      "setTimeout(() => addChatMessage(response, 'bot'), 500);"
      "input.value = '';"
    "}"
    
    "function atualizar() {"
      "fetch('/estado').then(res => res.json()).then(data => {"
        "lastValues = { temp_bmp: data.led, altitude: data.x, pressure: data.y, temp_aht: data.botao, humidity: data.joy };"
        "Object.keys(lastValues).forEach(key => {"
          "const el = document.getElementById(key.split('_')[0]);"
          "if(el) el.textContent = (typeof lastValues[key] === 'number') ? lastValues[key].toFixed(key.includes('pressao') ? 2 : 1) : lastValues[key];"
        "});"
        "document.getElementById('umidadeAlert').style.display = data.joy >= data.limiteHumidade ? 'inline' : 'none';"
        "document.getElementById('altitudeAlert').style.display = data.x >= data.limiteAltitude ? 'inline' : 'none';"
        "updateChart(humidityChart, humidityData, data.joy);"
        "updateChart(altitudeChart, altitudeData, data.x);"
      "}).catch(err => console.error('Erro de conexão:', err));"
    "}"
    
    "function enviarConfig(endpoint, inputId) {"
      "fetch(`/${endpoint}?limite=${document.getElementById(inputId).value}`);"
      "return false;"
    "}"
    
    "window.addEventListener('load', () => {"
      "humidityChart = createChart(document.getElementById('humidityChart').getContext('2d'), 'Umidade (%)', '#009688', '#e0e0e0');"
      "altitudeChart = createChart(document.getElementById('altitudeChart').getContext('2d'), 'Altitude (m)', '#ff9800', '#e0e0e0');"
      "addChatMessage('Terminal de logs iniciado. Aguardando comandos...', 'bot');"
      "setInterval(atualizar, 2000);"
      "atualizar();"
    "});"
  "</script>"
"</body>"
"</html>";


// ===============================================
//         CONFIGURAÇÕES E CONSTANTES
// ===============================================
#define I2C_PORT_SENSORS i2c0
#define I2C_SDA_SENSORS 0
#define I2C_SCL_SENSORS 1
#define I2C_PORT_DISP i2c1
#define I2C_SDA_DISP 14
#define I2C_SCL_DISP 15
#define DISPLAY_ADDR 0x3C
#define LED_PIN 13
#define BOTAO_A 5
#define BOTAO_B 6
#define BUZZER_A 21
#define BUZZER_B 10
#define WS2812_PIN 7
#define NUM_PIXELS 25
#define BUZZER_FREQUENCY 5000
#define SEA_LEVEL_PRESSURE 101325.0

// ===============================================
//   DEFINIÇÃO DO PADRÃO DE ALERTA (MODIFICADO)
// ===============================================
// Padrão de alerta para a matriz de LED (formato 5x5, mais legível)
// `true` = LED aceso, `false` = LED apagado.
// O padrão abaixo desenha um ponto de exclamação.
const bool PATTERN_ALERT[NUM_PIXELS] = {
    false, false, true, false, false,
    false, false, false, false, false,
    false, false, true, false, false,
    false, false, true, false, false,
    false, false, true, false, false
};


// ===============================================
//         ESTRUTURAS E VARIÁVEIS GLOBAIS
// ===============================================
float temp_bmp280, pres_bmp280, temp_ath20, hum_ath20;
int alt_bmp280;
int limite_humidade = 80;
int calibra_altitude = 0;
int limite_altitude = 800;
float calibra_umidade = 0.0;

struct http_state {
    char response[16384];
    size_t len;
    size_t sent;
};

// ===============================================
//         FUNÇÕES DO SERVIDOR WEB
// ===============================================
err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    struct http_state *hs = (struct http_state *)arg;
    hs->sent += len;
    if (hs->sent >= hs->len) {
        tcp_close(tpcb);
        free(hs);
    }
    return ERR_OK;
}

int get_param_val(const char *query, const char *key) {
    const char *p = strstr(query, key);
    if (!p) return -1;
    p += strlen(key);
    if (*p != '=') return -1;
    return atoi(p + 1);
}

err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        return ERR_OK;
    }
    char *req = (char *)p->payload;
    struct http_state *hs = malloc(sizeof(struct http_state));
    if (!hs) {
        pbuf_free(p);
        tcp_close(tpcb);
        return ERR_MEM;
    }
    hs->sent = 0;
    if (strstr(req, "GET /estado")) {
        char json_payload[200];
        int json_len = snprintf(json_payload, sizeof(json_payload),
                                "{\"led\":%.1f,\"x\":%d,\"y\":%.1f,\"botao\":%.1f,\"joy\":%.1f,"
                                "\"limiteHumidade\":%d,\"limiteAltitude\":%d}\r\n",
                                temp_bmp280, alt_bmp280, pres_bmp280, temp_ath20, hum_ath20,
                                limite_humidade, limite_altitude);
        hs->len = snprintf(hs->response, sizeof(hs->response),
                             "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s",
                             json_len, json_payload);
    } else if (strncmp(req, "GET /configmax", 14) == 0) {
        limite_humidade = get_param_val(req, "limite");
        hs->len = snprintf(hs->response, sizeof(hs->response), "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK");
    } else if (strncmp(req, "GET /configmin", 14) == 0) {
        calibra_altitude = get_param_val(req, "limite");
        hs->len = snprintf(hs->response, sizeof(hs->response), "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK");
    } else if (strncmp(req, "GET /configalt", 14) == 0) {
        limite_altitude = get_param_val(req, "limite");
        hs->len = snprintf(hs->response, sizeof(hs->response), "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK");
    } else if (strncmp(req, "GET /configumi", 14) == 0) {
        calibra_umidade = (float)get_param_val(req, "limite") / 10.0;
        hs->len = snprintf(hs->response, sizeof(hs->response), "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK");
    } else {
        hs->len = snprintf(hs->response, sizeof(hs->response),
                             "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s",
                             (int)strlen(HTML_BODY), HTML_BODY);
    }
    tcp_arg(tpcb, hs);
    tcp_sent(tpcb, http_sent);
    tcp_write(tpcb, hs->response, hs->len, TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    pbuf_free(p);
    return ERR_OK;
}

err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

void start_http_server(void) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb || tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao iniciar o servidor HTTP.\n");
        return;
    }
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, connection_callback);
    printf("Servidor HTTP rodando na porta 80...\n");
}

// ===============================================
//         FUNÇÕES PARA BUZZER
// ===============================================
void initPwm() {
    gpio_set_function(BUZZER_A, GPIO_FUNC_PWM);
    gpio_set_function(BUZZER_B, GPIO_FUNC_PWM);
    uint slice_num_A = pwm_gpio_to_slice_num(BUZZER_A);
    uint slice_num_B = pwm_gpio_to_slice_num(BUZZER_B);
    pwm_set_gpio_level(BUZZER_A, 0);
    pwm_set_gpio_level(BUZZER_B, 0);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096.f));
    pwm_init(slice_num_A, &config, true);
    pwm_init(slice_num_B, &config, true);
}

void beep(uint16_t wrap) {
    uint slice_a = pwm_gpio_to_slice_num(BUZZER_A);
    uint slice_b = pwm_gpio_to_slice_num(BUZZER_B);
    pwm_set_wrap(slice_a, wrap);
    pwm_set_wrap(slice_b, wrap);
    pwm_set_gpio_level(BUZZER_A, wrap / 2);
    pwm_set_gpio_level(BUZZER_B, wrap / 2);
    pwm_set_enabled(slice_a, true);
    pwm_set_enabled(slice_b, true);
}

void semSom() {
    pwm_set_gpio_level(BUZZER_A, 0);
    pwm_set_gpio_level(BUZZER_B, 0);
}

// ===============================================
//   FUNÇÕES PARA MATRIZ LED (MODIFICADAS)
// ===============================================
PIO pio_matrix = pio0;
uint sm_matrix = 0;
typedef struct { uint8_t G, R, B; } pixel_t;
pixel_t leds[NUM_PIXELS];

void npInit() {
    uint offset = pio_add_program(pio_matrix, &ws2812_program);
    sm_matrix = pio_claim_unused_sm(pio_matrix, true);
    ws2812_program_init(pio_matrix, sm_matrix, offset, WS2812_PIN, 800000, false);
}

void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

void npWrite() {
    for (uint i = 0; i < NUM_PIXELS; ++i) {
        pio_sm_put_blocking(pio_matrix, sm_matrix, ((uint32_t)leds[i].G << 16) | ((uint32_t)leds[i].R << 8) | (uint32_t)leds[i].B);
    }
}

void npClear() {
    for (uint i = 0; i < NUM_PIXELS; ++i) npSetLED(i, 0, 0, 0);
    npWrite();
}

// NOVA FUNÇÃO: Exibe um padrão booleano com uma cor específica
void npShowPattern(const bool pattern[NUM_PIXELS], uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        if (pattern[i]) {
            npSetLED(i, r, g, b);
        } else {
            npSetLED(i, 0, 0, 0);
        }
    }
    npWrite();
}


// ===============================================
//         FUNÇÃO DE CÁLCULO DE ALTITUDE
// ===============================================
double calculate_altitude(double pressure) {
    return 44330.0 * (1.0 - pow(pressure / SEA_LEVEL_PRESSURE, 0.1903));
}

// ===============================================
//         FUNÇÃO PRINCIPAL (MAIN)
// ===============================================
volatile uint32_t last_button_time = 0;

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_button_time < 200000) return;
    last_button_time = current_time;
    if (gpio == BOTAO_B) {
        reset_usb_boot(0, 0);
    } else if (gpio == BOTAO_A) {
        limite_humidade = 80;
        calibra_altitude = 0;
        limite_altitude = 800;
        calibra_umidade = 0.0;
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    // --- INICIALIZAÇÃO DE PERIFÉRICOS ---
    i2c_init(I2C_PORT_DISP, 400 * 1000);
    gpio_set_function(I2C_SDA_DISP, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_DISP, GPIO_FUNC_I2C);
    
    i2c_init(I2C_PORT_SENSORS, 400 * 1000);
    gpio_set_function(I2C_SDA_SENSORS, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_SENSORS, GPIO_FUNC_I2C);
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
    // Inicialização do Buzzer e Matriz
    initPwm();
    npInit();
    npClear();
    
    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, DISPLAY_ADDR, I2C_PORT_DISP);
    ssd1306_config(&ssd);
    
    // --- INICIALIZAÇÃO DOS SENSORES ---
    bmp280_init(I2C_PORT_SENSORS);
    struct bmp280_calib_param params;
    bmp280_get_calib_params(I2C_PORT_SENSORS, &params);
    aht20_reset(I2C_PORT_SENSORS);
    aht20_init(I2C_PORT_SENSORS);

    // --- CONEXÃO WI-FI ---
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, "Iniciando WiFi...", 0, 15);
    ssd1306_send_data(&ssd);
    if (cyw43_arch_init()) { return 1; }
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) { return 1; }

    // --- SERVIDOR WEB E LOOP PRINCIPAL ---
    char ip_str[24];
    snprintf(ip_str, sizeof(ip_str), "%s", ip4addr_ntoa(netif_ip4_addr(netif_default)));
    start_http_server();

    uint32_t last_sensor_read_time = 0;
    const uint32_t sensor_read_interval_ms = 500;
    AHT20_Data data;
    int32_t raw_temp_bmp, raw_pressure;
    char str_tmp1[10], str_alt[10], str_tmp2[10], str_umi[10];
    int alert_frame = 0;

    while (true) {
        cyw43_arch_poll();

        uint32_t current_time_ms = to_ms_since_boot(get_absolute_time());
        if (current_time_ms - last_sensor_read_time >= sensor_read_interval_ms) {
            
            last_sensor_read_time = current_time_ms;

            // Leitura dos Sensores
            bmp280_read_raw(I2C_PORT_SENSORS, &raw_temp_bmp, &raw_pressure);
            int32_t temperature = bmp280_convert_temp(raw_temp_bmp, &params);
            int32_t pressure = bmp280_convert_pressure(raw_pressure, raw_temp_bmp, &params);
            double altitude = calculate_altitude(pressure) + calibra_altitude;
            aht20_read(I2C_PORT_SENSORS, &data);

            // Atualização das variáveis globais
            temp_bmp280 = temperature / 100.0;
            pres_bmp280 = pressure / 1000.0;
            alt_bmp280 = (int)altitude;
            temp_ath20 = data.temperature;
            hum_ath20 = data.humidity + calibra_umidade;

            // Lógica de Alerta (MODIFICADA)
            if (hum_ath20 >= limite_humidade || alt_bmp280 >= limite_altitude) {
                beep(6000);
                
                // A animação agora pisca o padrão de alerta em vermelho
                if (alert_frame % 2 == 0) {
                    // Frame par: mostra o padrão em vermelho (R=255, G=0, B=0)
                    npShowPattern(PATTERN_ALERT, 255, 0, 0);
                } else {
                    // Frame ímpar: apaga a matriz
                    npClear();
                }
                
                gpio_put(LED_PIN, alert_frame % 2); // Mantém o piscar do LED onboard
                alert_frame++; // Incrementa para alternar os frames
            } else {
                semSom();
                npClear();
                gpio_put(LED_PIN, 0);
                alert_frame = 0; // Reseta o contador de frames quando não há alerta
            }

            // Atualização do Display OLED
            sprintf(str_tmp1, "%.1fC", temp_bmp280);
            sprintf(str_alt, "%.0fm", (float)alt_bmp280);
            sprintf(str_tmp2, "%.1fC", temp_ath20);
            sprintf(str_umi, "%.1f%%", hum_ath20);

            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, "Est. Meteoro", 15, 6);
            ssd1306_draw_string(&ssd, ip_str, 12, 16);
            ssd1306_draw_string(&ssd, "BMP280  AHT20", 10, 28);
            ssd1306_line(&ssd, 63, 25, 63, 60, true);
            ssd1306_draw_string(&ssd, str_tmp1, 14, 41);
            ssd1306_draw_string(&ssd, str_alt, 14, 52);
            ssd1306_draw_string(&ssd, str_tmp2, 73, 41);
            ssd1306_draw_string(&ssd, str_umi, 73, 52);
            ssd1306_send_data(&ssd);
        }
    }
    
    cyw43_arch_deinit();
    return 0;
}