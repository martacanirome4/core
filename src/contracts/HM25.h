#include  "qpi.h"
using namespace QPI;


// Definición del contrato que hereda de ContractBase
struct TRANSFERENCIA_CONDICIONAL : public ContractBase
{
    // Estructura para almacenar los detalles de una transferencia
    struct Transferencia
    {
        uint64 id;              // ID único de la transferencia
        uint64 nombre;          // Nombre/identificador del activo
        sint64 cantidad;        // Cantidad a transferir
        id emisor;              // Wallet del emisor
        id receptor;            // Wallet del receptor
        bit aprobadaPorReceptor;// Indica si el receptor ha aprobado
        bit completada;         // Indica si la transferencia se ha completado
    };

    // Estructuras de entrada/salida para los procedimientos
    struct CrearTransferencia_input
    {
        uint64 nombre;          // Nombre del activo a transferir
        sint64 cantidad;        // Cantidad a transferir
        id receptor;            // Wallet del destinatario
    };
    
    struct CrearTransferencia_output
    {
        uint64 id;              // ID de la transferencia creada
        bit exito;              // Indica si la creación fue exitosa
    };

    struct AprobarTransferencia_input
    {
        uint64 id;              // ID de la transferencia a aprobar
    };
    
    struct AprobarTransferencia_output
    {
        bit exito;              // Indica si la aprobación fue exitosa
    };

    struct EjecutarTransferencia_input
    {
        uint64 id;              // ID de la transferencia a ejecutar
    };
    
    struct EjecutarTransferencia_output
    {
        bit exito;              // Indica si la ejecución fue exitosa
    };

    struct ConsultarTransferencia_input
    {
        uint64 id;              // ID de la transferencia a consultar
    };
    
    struct ConsultarTransferencia_output
    {
        uint64 nombre;          // Nombre del activo
        sint64 cantidad;        // Cantidad a transferir
        id emisor;              // Wallet del emisor
        id receptor;            // Wallet del receptor
        bit aprobadaPorReceptor;// Estado de aprobación
        bit completada;         // Estado de completitud
        bit existe;             // Indica si la transferencia existe
    };

    // Agregar estas estructuras para ser compatibles con HM25
    struct Echo_input{};
    struct Echo_output{};
    struct Burn_input{};
    struct Burn_output{};
    struct GetStats_input {};
    struct GetStats_output
    {
        uint64 numberOfEchoCalls;
        uint64 numberOfBurnCalls;
    };

protected:
    // Variables de estado del contrato
    Array<Transferencia, 100> transferencias;   // Almacena hasta 100 transferencias
    uint64 contadorTransferencias = 0;          // Contador para asignar IDs únicos, inicializado a 0
    
    // Variables adicionales del template HM25
    uint64 numberOfEchoCalls;
    uint64 numberOfBurnCalls;

    // Implementación de los procedimientos públicos
    PUBLIC_PROCEDURE(CrearTransferencia)
        // Verificar que el emisor tiene los fondos necesarios
        Asset asset;
        asset.assetName = input.nombre;
        asset.issuer = qpi.invocator(); // El emisor es quien invoca el contrato
        
        // Verificar que hay espacio para una nueva transferencia
        if (state.contadorTransferencias < 100) {
            // Crear nueva transferencia
            Transferencia nuevaTransferencia;
            nuevaTransferencia.id = state.contadorTransferencias + 1;
            nuevaTransferencia.nombre = input.nombre;
            nuevaTransferencia.cantidad = input.cantidad;
            nuevaTransferencia.emisor = qpi.invocator();
            nuevaTransferencia.receptor = input.receptor;
            nuevaTransferencia.aprobadaPorReceptor = false;
            nuevaTransferencia.completada = false;
            
            // Guardar la transferencia en el estado
            state.transferencias[state.contadorTransferencias] = nuevaTransferencia;
            state.contadorTransferencias++;
            
            // Preparar respuesta
            output.id = nuevaTransferencia.id;
            output.exito = true;
        } else {
            // No hay espacio para más transferencias
            output.id = 0;
            output.exito = false;
        }
    _

    PUBLIC_PROCEDURE(AprobarTransferencia)
        output.exito = false;
        
        // Buscar la transferencia por ID
        for (uint64 i = 0; i < state.contadorTransferencias; i++) {
            if (state.transferencias[i].id == input.id) {
                // Verificar que quien invoca es el receptor
                if (state.transferencias[i].receptor == qpi.invocator()) {
                    // Verificar que la transferencia no esté ya completada
                    if (!state.transferencias[i].completada) {
                        // Aprobar la transferencia
                        state.transferencias[i].aprobadaPorReceptor = true;
                        output.exito = true;
                    }
                }
                break;
            }
        }
    _

    PUBLIC_PROCEDURE(EjecutarTransferencia)
        output.exito = false;
        
        // Buscar la transferencia por ID
        for (uint64 i = 0; i < state.contadorTransferencias; i++) {
            if (state.transferencias[i].id == input.id) {
                // Verificar que quien invoca es el emisor
                if (state.transferencias[i].emisor == qpi.invocator()) {
                    // Verificar que la transferencia está aprobada pero no completada
                    if (state.transferencias[i].aprobadaPorReceptor && !state.transferencias[i].completada) {
                        // Realizar la transferencia del activo
                        Asset asset;
                        asset.assetName = state.transferencias[i].nombre;
                        asset.issuer = state.transferencias[i].emisor;
                        
                        // Transferir la propiedad y posesión
                        if (qpi.transferShareOwnershipAndPossession(
                            asset.assetName, 
                            asset.issuer, 
                            state.transferencias[i].emisor, 
                            state.transferencias[i].emisor, 
                            state.transferencias[i].cantidad, 
                            state.transferencias[i].receptor) > 0) {
                            
                            // Marcar como completada
                            state.transferencias[i].completada = true;
                            output.exito = true;
                        }
                    }
                }
                break;
            }
        }
    _

    PUBLIC_PROCEDURE(ConsultarTransferencia)
        output.existe = false;
        
        // Buscar la transferencia por ID
        for (uint64 i = 0; i < state.contadorTransferencias; i++) {
            if (state.transferencias[i].id == input.id) {
                // Copiar los datos a la salida
                output.nombre = state.transferencias[i].nombre;
                output.cantidad = state.transferencias[i].cantidad;
                output.emisor = state.transferencias[i].emisor;
                output.receptor = state.transferencias[i].receptor;
                output.aprobadaPorReceptor = state.transferencias[i].aprobadaPorReceptor;
                output.completada = state.transferencias[i].completada;
                output.existe = true;
                break;
            }
        }
    _

    /**
    Send back the invocation amount
    */
    PUBLIC_PROCEDURE(Echo)
        state.numberOfEchoCalls++;
        if (qpi.invocationReward() > 0)
        {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
        }
    _
    
    /**
    * Burn all invocation amount
    */
    PUBLIC_PROCEDURE(Burn)
        state.numberOfBurnCalls++;
        if (qpi.invocationReward() > 0)
        {
            qpi.burn(qpi.invocationReward());
        }
    _
    
    PUBLIC_FUNCTION(GetStats)
        output.numberOfBurnCalls = state.numberOfBurnCalls;
        output.numberOfEchoCalls = state.numberOfEchoCalls;
    _

    // Registro de procedimientos públicos
    REGISTER_USER_FUNCTIONS_AND_PROCEDURES
        REGISTER_USER_PROCEDURE(CrearTransferencia, 1);
        REGISTER_USER_PROCEDURE(AprobarTransferencia, 2);
        REGISTER_USER_PROCEDURE(EjecutarTransferencia, 3);
        REGISTER_USER_PROCEDURE(ConsultarTransferencia, 4);
        REGISTER_USER_PROCEDURE(Echo, 5);
        REGISTER_USER_PROCEDURE(Burn, 6);
        REGISTER_USER_FUNCTION(GetStats, 1);
    _
    
    // Inicialización de las variables de estado
    INITIALIZE
        state.contadorTransferencias = 0;
        state.numberOfEchoCalls = 0;
        state.numberOfBurnCalls = 0;
    _
};
message.txt
10 KB