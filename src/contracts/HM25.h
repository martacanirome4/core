#include <qpi.h>
using namespace QPI;

struct HM252
{
};

struct HM25 : public ContractBase
{
public:
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

    // Input/Output para Deposit
    struct Deposit_input {
        string providerName;
        Identity counterparty;            // wallet_address
        Identity validator;
        string clause1;
        string clause2;
        string clause3;
        string productName;
        uint64 quantity;
        uint64 pricePerUnit;
        uint64 totalPrice;
        uint64 deliveryDeadlineEpoch;     // delivery_deadline
        uint64 contractStartEpoch;        // contract_start_date
    };
    struct Deposit_output {};

    struct Withdraw_input {};
    struct Withdraw_output {};

    struct Validate_input {};
    struct Validate_output {};

    struct GetDetails_input {};
    struct GetDetails_output {
        string providerName;
        Identity creator;
        Identity counterparty;
        Identity validator;
        string productName;
        uint64 quantity;
        uint64 pricePerUnit;
        uint64 totalPrice;
        uint64 deliveryDeadlineEpoch;
        uint64 contractStartEpoch;
        string clause1;
        string clause2;
        string clause3;
        bool isActive;
        bool isValidated;
    };

private:
    // ==== Estado original ====
    uint64 numberOfEchoCalls;
    uint64 numberOfBurnCalls;

    string providerName;
    Identity counterparty;
    Identity validator;
    string productName;
    uint64 quantity;
    uint64 pricePerUnit;
    uint64 totalPrice;
    uint64 deliveryDeadlineEpoch;
    uint64 contractStartEpoch;
    string clause1;
    string clause2;
    string clause3;
    bool isActive;
    bool isValidated;

    // ==== Procedimientos originales ====
    PUBLIC_PROCEDURE(Echo)
        state.numberOfEchoCalls++;
        if (qpi.invocationReward() > 0)
        {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
        }
    _

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

    // ==== Nuevos procedimientos personalizados ====
    PUBLIC_PROCEDURE(Deposit)
        if (state.isActive) return;
        if (qpi.invocationReward() <= 0) return;

        state.providerName = input.providerName;
        state.counterparty = input.counterparty;
        state.validator = input.validator;
        state.productName = input.productName;
        state.quantity = input.quantity;
        state.pricePerUnit = input.pricePerUnit;
        state.totalPrice = qpi.invocationReward(); // Confirmamos que es igual al enviado
        state.deliveryDeadlineEpoch = input.deliveryDeadlineEpoch;
        state.contractStartEpoch = input.contractStartEpoch;
        state.clause1 = input.clause1;
        state.clause2 = input.clause2;
        state.clause3 = input.clause3;
        state.isActive = true;
        state.isValidated = false;
    _


    PUBLIC_PROCEDURE(Validate)
        if (!state.isActive) return;
        if (qpi.invocator() != state.validator) return;

        state.isValidated = true;
    _

    PUBLIC_PROCEDURE(Withdraw)
        if (!state.isActive || !state.isValidated) return;
        if (qpi.invocator() != state.counterparty) return;

        uint64 payout = state.totalPrice;
        // Penalización si hay retraso
        uint64 currentEpoch = qpi.epochTime(); // Hora actual en epoch
        if (currentEpoch > state.deliveryDeadlineEpoch) {
            uint64 penalty = state.totalPrice / 10;
            payout -= penalty;
        }

        qpi.transfer(state.counterparty, payout);
        state.isActive = false;
    _

    PUBLIC_FUNCTION(GetContractDetails)
        output.providerName = state.providerName;
        output.creator = qpi.contractOwner();
        output.counterparty = state.counterparty;
        output.validator = state.validator;
        output.productName = state.productName;
        output.quantity = state.quantity;
        output.pricePerUnit = state.pricePerUnit;
        output.totalPrice = state.totalPrice;
        output.deliveryDeadlineEpoch = state.deliveryDeadlineEpoch;
        output.contractStartEpoch = state.contractStartEpoch;
        output.clause1 = state.clause1;
        output.clause2 = state.clause2;
        output.clause3 = state.clause3;
        output.isActive = state.isActive;
        output.isValidated = state.isValidated;
    _


    // ==== Registro único de todas las funciones ====
    REGISTER_USER_FUNCTIONS_AND_PROCEDURES
        REGISTER_USER_PROCEDURE(Echo, 1);
        REGISTER_USER_PROCEDURE(Burn, 2);
        REGISTER_USER_PROCEDURE(Deposit, 3);
        REGISTER_USER_PROCEDURE(Validate, 4);
        REGISTER_USER_PROCEDURE(Withdraw, 5);
        REGISTER_USER_FUNCTION(GetStats, 1);
        REGISTER_USER_FUNCTION(GetContractDetails, 2);
    _

    INITIALIZE
        state.numberOfEchoCalls = 0;
        state.numberOfBurnCalls = 0;
        state.isActive = false;
        state.isValidated = false;
        state.totalPrice = 0;
    _
};
