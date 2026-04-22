PenStream Protocol Specification (Snapshot)

Overview
- PenStream transmite video y control/entrada desde Windows a Android usando UDP. Los paquetes se componen de un encabezado fijo seguido de datos de payload variable.

Packet Header (ejemplo)
- uint16_t packet_type: identifica la naturaleza del paquete (ej. I_FRAME, P_FRAME, CONTROL, HEARTBEAT, etc).
- uint16_t seq: número de secuencia para detección de pérdida y reorder.
- uint32_t timestamp: marca de tiempo para sincronización entre emisor y receptor.
- uint32_t payload_len: longitud en bytes del payload que sigue al encabezado.

Payload
- El formato del payload depende del packet_type.
- Para video (FRAME packets): payload contiene NAL units o fragmentos de frame ya empaquetados (p. ej. H.264 NALUs) listos para decodificar.
- Para CONTROL/INPUT: payload puede contener comandos de entrada, respuestas de estado, o configuración.
- Para HEARTBEAT: payload vacío o pequeño footprint de estado mínimo.

Notas
- Los tamaños de payload deben ajustarse al MTU para evitar fragmentation excesiva. En redes con pérdidas, se recomienda un tamaño de payload moderado (p. ej. 1200-1400 bytes).
- Cualquier cambio en este protocolo debe actualizar protocol_spec.md y los archivos de código relevantes que implementan el empaquetado y desempaquetado.

Versiones y compatibilidad
- Este documento describe la versión actual del protocolo. Cambios incompatibles deben introducirse con una nueva versión de protocolo y/o un mapa de compatibilidad.
