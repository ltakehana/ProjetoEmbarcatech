import re
import sys
import time
import signal
import logging
import requests
import serial

# Configurações
API_URL = "http://localhost:8000/data"  # URL da API (ajuste se necessário)
SERIAL_PORT = "/dev/ttyACM0"              # Porta serial (ajuste para sua realidade)
BAUD_RATE = 9600                        # Taxa de transmissão (baud)

# Configuração do logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s"
)

# Expressão regular para extrair os dados da linha
pattern = (
    r"Setpoint Corrente\s*=\s*([\d.]+)\s*A;\s*"
    r"Medida Corrente\s*=\s*([\d.]+)\s*A;\s*"
    r"Ativo\s*=\s*(\d+);\s*Modo:\s*([A-Z]{2});\s*PWM\s*=\s*(\d+)"
)

# Variável para controle do loop
running = True

def signal_handler(sig, frame):
    global running
    logging.info("Recebido sinal de encerramento. Finalizando daemon...")
    running = False

# Registra tratamento de sinais para encerramento gracioso
signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

def main():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    except Exception as e:
        logging.error("Erro ao abrir a porta serial %s: %s", SERIAL_PORT, e)
        sys.exit(1)

    logging.info("Daemon iniciado. Lendo dados da porta %s.", SERIAL_PORT)

    while running:
        try:
            # Lê uma linha da porta serial
            line = ser.readline().decode("utf-8").strip()
            if not line:
                continue

            logging.info("Linha recebida: %s", line)

            # Tenta extrair os dados usando a regex
            m = re.search(pattern, line)
            if m:
                currentSetpoint = float(m.group(1))
                currentMeasured = float(m.group(2))
                active = bool(int(m.group(3)))
                mode = m.group(4)
                pwm = int(m.group(5))

                data = {
                    "currentSetpoint": currentSetpoint,
                    "currentMeasured": currentMeasured,
                    "active": active,
                    "mode": mode,
                    "pwm": pwm
                }

                # Envia os dados para a API
                response = requests.post(API_URL, json=data)
                if response.status_code == 200:
                    logging.info("Dados enviados com sucesso.")
                else:
                    logging.error("Falha ao enviar dados: %s", response.text)
            else:
                logging.warning("Formato da linha não reconhecido: %s", line)
        except Exception as e:
            logging.error("Erro durante leitura ou envio: %s", e)

        # Pausa breve antes da próxima leitura
        time.sleep(1)

    ser.close()
    logging.info("Daemon finalizado.")

if __name__ == "__main__":
    main()
