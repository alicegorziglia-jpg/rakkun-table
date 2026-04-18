package com.graphictablet

import kotlinx.coroutines.*
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress

/**
 * Broadcastea la presencia del dispositivo en la red local cada 2 segundos.
 * El server Windows escucha este broadcast y obtiene la IP automáticamente.
 *
 * Protocolo: UDP broadcast → 255.255.255.255:9001
 * Payload: "GRAPHICTABLET_HELLO:<ip_local>:<udp_port>"
 */
class DiscoveryService(
    private val localIp: String,
    private val streamPort: Int = 9000,
    private val discoveryPort: Int = 9001
) {
    private var job: Job? = null

    fun start(scope: CoroutineScope, onServerFound: (serverIp: String) -> Unit) {
        job = scope.launch(Dispatchers.IO) {

            // Hilo 1: broadcaster (anuncia nuestra presencia)
            launch { broadcastPresence() }

            // Hilo 2: listener (escucha si el server nos responde con su IP)
            launch { listenForServer(onServerFound) }
        }
    }

    fun stop() {
        job?.cancel()
    }

    private suspend fun broadcastPresence() {
        val socket = DatagramSocket()
        socket.broadcast = true

        val message = "GRAPHICTABLET_HELLO:$localIp:$streamPort"
        val data = message.toByteArray()
        val broadcast = InetAddress.getByName("255.255.255.255")

        try {
            while (true) {
                val packet = DatagramPacket(data, data.size, broadcast, discoveryPort)
                socket.send(packet)
                delay(2000) // broadcast cada 2 segundos
            }
        } catch (e: Exception) {
            // Coroutine cancelada o error de red — salir limpio
        } finally {
            socket.close()
        }
    }

    private suspend fun listenForServer(onServerFound: (String) -> Unit) {
        val socket = DatagramSocket(discoveryPort + 1) // puerto 9002
        val buffer = ByteArray(256)

        try {
            while (true) {
                val packet = DatagramPacket(buffer, buffer.size)
                withContext(Dispatchers.IO) {
                    socket.receive(packet)
                }
                val message = String(packet.data, 0, packet.length)
                if (message.startsWith("GRAPHICTABLET_ACK:")) {
                    val serverIp = message.removePrefix("GRAPHICTABLET_ACK:").trim()
                    withContext(Dispatchers.Main) {
                        onServerFound(serverIp)
                    }
                }
            }
        } catch (e: Exception) {
            // Cancelado
        } finally {
            socket.close()
        }
    }
}
