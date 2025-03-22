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
        uint64 providerName;
        uint64 counterparty;            // wallet_address
        uint64 validator;
        uint64 clause1;
        uint64 clause2;
        uint64 clause3;
        uint64 productName;
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
        uint64 providerName;
        uint64 creator;
        uint64 counterparty;
        uint64 validator;
        uint64 productName;
        uint64 quantity;
        uint64 pricePerUnit;
        uint64 totalPrice;
        uint64 deliveryDeadlineEpoch;
        uint64 contractStartEpoch;
        uint64 clause1;
        uint64 clause2;
        uint64 clause3;
        bool isActive;
        bool isValidated;
    };

private:
    uint64 numberOfEchoCalls;
    uint64 numberOfBurnCalls;

    struct StateData {
        uint64 providerName;
        uint64 counterparty;
        uint64 validator;
        uint64 clause1;
        uint64 clause2;
        uint64 clause3;
        uint64 productName;
        uint64 quantity;
        uint64 pricePerUnit;
        uint64 totalPrice;
        uint64 deliveryDeadlineEpoch;
        uint64 contractStartEpoch;
        bool isActive;
        bool isValidated;
    };

    StateData state;

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

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES

        REGISTER_USER_PROCEDURE(Echo, 1);
        REGISTER_USER_PROCEDURE(Burn, 2);

        REGISTER_USER_FUNCTION(GetStats, 1);
    
    _

    INITIALIZE
        state.numberOfEchoCalls = 0;
        state.numberOfBurnCalls = 0;
    _

};
