"""
Auto-discovery: escucha broadcasts del Android y obtiene su IP automáticamente.
El Android broadcastea "GRAPHICTABLET_HELLO:<ip>:<port>" cada 2 segundos.
El server responde con "GRAPHICTABLET_ACK:<server_ip>" y retorna la IP del cliente.
"""

import socket
import threading
import time


DISCOVERY_PORT = 9001  # Puerto donde escuchamos broadcasts del Android
ACK_PORT = 9002        # Puerto donde enviamos el ACK al Android
HELLO_PREFIX = "GRAPHICTABLET_HELLO:"
ACK_PREFIX = "GRAPHICTABLET_ACK:"


def get_local_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        return s.getsockname()[0]
    except Exception:
        return "127.0.0.1"
    finally:
        s.close()


def wait_for_android(timeout=60, on_found=None):
    """
    Espera a que el Android se anuncie en la red.
    
    Args:
        timeout: segundos máximos de espera (default 60)
        on_found: callback(android_ip, stream_port) cuando se detecta
    
    Returns:
        (android_ip, stream_port) o (None, None) si timeout
    """
    local_ip = get_local_ip()
    result = [None, None]
    found_event = threading.Event()

    def listener():
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.settimeout(2.0)

        try:
            sock.bind(("", DISCOVERY_PORT))

            while not found_event.is_set():
                try:
                    data, addr = sock.recvfrom(256)
                    message = data.decode("utf-8").strip()

                    if message.startswith(HELLO_PREFIX):
                        # Parsear IP y puerto del mensaje
                        payload = message[len(HELLO_PREFIX):]
                        parts = payload.split(":")
                        android_ip = parts[0] if parts else addr[0]
                        stream_port = int(parts[1]) if len(parts) > 1 else 9000

                        # Responder con ACK
                        ack_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                        ack_message = f"{ACK_PREFIX}{local_ip}".encode("utf-8")
                        ack_sock.sendto(ack_message, (android_ip, ACK_PORT))
                        ack_sock.close()

                        result[0] = android_ip
                        result[1] = stream_port
                        found_event.set()

                        if on_found:
                            on_found(android_ip, stream_port)

                except socket.timeout:
                    continue

        except Exception as e:
            print(f"  [Discovery] Error: {e}")
        finally:
            sock.close()

    thread = threading.Thread(target=listener, daemon=True)
    thread.start()
    found_event.wait(timeout=timeout)

    return result[0], result[1]


def discover_android_interactive():
    """
    Versión interactiva: imprime spinners y mensajes, retorna (ip, port).
    """
    local_ip = get_local_ip()
    spinners = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"]
    found = threading.Event()
    result = [None, None]

    def on_found(ip, port):
        result[0] = ip
        result[1] = port
        found.set()

    listener_thread = threading.Thread(
        target=lambda: wait_for_android(timeout=120, on_found=on_found),
        daemon=True
    )
    listener_thread.start()

    print(f"\n  Esperando que el Android se conecte...")
    print(f"  (Abrí la app GraphicTablet en tu teléfono)\n")

    i = 0
    while not found.is_set():
        print(f"\r  {spinners[i % len(spinners)]}  Escuchando en red local ({local_ip})...", end="", flush=True)
        i += 1
        time.sleep(0.1)

    print(f"\r  ✓  Android encontrado: {result[0]}:{result[1]}           ")
    return result[0], result[1]
