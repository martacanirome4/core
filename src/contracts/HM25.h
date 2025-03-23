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

    // WalletAgreement State Variables
    id partyA;
    id partyB;
    id agreementName;
    id agreementType;

    uint64 depositAmount;
    uint32 startDate;
    uint32 paymentPeriodDays;
    uint32 lastPaymentTick;
    bit active;

    // WalletAgreement I/O Structs
    struct InitAgreement_input
    {
        id agreementName;
        id agreementType;
        id partyB;
        uint64 depositAmount;
        uint32 paymentPeriodDays;
    };

    typedef NoData InitAgreement_output;

    struct Deposit_input { };
    typedef NoData Deposit_output;

    struct Withdraw_input { };
    typedef NoData Withdraw_output;

    struct GetAgreementDetails_input { };
    struct GetAgreementDetails_output
    {
        id partyA;
        id partyB;
        id agreementName;
        id agreementType;
        uint64 depositAmount;
        uint32 startDate;
        uint32 paymentPeriodDays;
        uint32 lastPaymentTick;
        bit active;
    };

    struct CancelAgreement_input { };
    typedef NoData CancelAgreement_output;

private:
    uint64 numberOfEchoCalls;
    uint64 numberOfBurnCalls;

    // Echo Procedure
    PUBLIC_PROCEDURE(Echo)
        state.numberOfEchoCalls++;
        if (qpi.invocationReward() > 0)
        {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
        }
    _

    // Burn Procedure
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

    // WalletAgreement Procedures

    PUBLIC_PROCEDURE(InitAgreement)
        if (state.active)
        {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            return;
        }
        state.partyA = qpi.invocator();
        state.partyB = input.partyB;
        state.agreementName = input.agreementName;
        state.agreementType = input.agreementType;
        state.depositAmount = input.depositAmount;
        getCurrentDate(qpi, state.startDate);
        state.paymentPeriodDays = input.paymentPeriodDays;
        state.lastPaymentTick = qpi.tick();
        state.active = 1;

        if (qpi.invocationReward() > 0)
        {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
        }
    _

    PUBLIC_PROCEDURE(Deposit)
        if (qpi.invocator() != state.partyA || !state.active)
        {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            return;
        }
        state.depositAmount += qpi.invocationReward();
        qpi.burn(0);
    _

    PUBLIC_PROCEDURE(Withdraw)
        if (qpi.invocator() != state.partyB || !state.active)
        {
            return;
        }

        uint32 ticksSinceLastPayment = qpi.tick() - state.lastPaymentTick;
        uint32 ticksPerPeriod = state.paymentPeriodDays * TICKS_PER_DAY;

        if (ticksSinceLastPayment < ticksPerPeriod)
        {
            return;
        }

        uint64 paymentAmount = state.depositAmount;

        if (paymentAmount > 0)
        {
            qpi.transfer(state.partyB, paymentAmount);
            state.depositAmount = 0;
            state.lastPaymentTick = qpi.tick();
        }
    _

    PUBLIC_PROCEDURE(CancelAgreement)
        if (!state.active || (qpi.invocator() != state.partyA && qpi.invocator() != state.partyB))
        {
            return;
        }

        if (state.depositAmount > 0)
        {
            qpi.transfer(state.partyA, state.depositAmount);
        }

        state.active = 0;
    _

    PUBLIC_FUNCTION(GetAgreementDetails)
        output.partyA = state.partyA;
        output.partyB = state.partyB;
        output.agreementName = state.agreementName;
        output.agreementType = state.agreementType;
        output.depositAmount = state.depositAmount;
        output.startDate = state.startDate;
        output.paymentPeriodDays = state.paymentPeriodDays;
        output.lastPaymentTick = state.lastPaymentTick;
        output.active = state.active;
    _

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES
        REGISTER_USER_PROCEDURE(Echo, 1);
        REGISTER_USER_PROCEDURE(Burn, 2);
        REGISTER_USER_FUNCTION(GetStats, 1);

        REGISTER_USER_PROCEDURE(InitAgreement, 3);
        REGISTER_USER_PROCEDURE(Deposit, 4);
        REGISTER_USER_PROCEDURE(Withdraw, 5);
        REGISTER_USER_PROCEDURE(CancelAgreement, 6);
        REGISTER_USER_FUNCTION(GetAgreementDetails, 2);
    _

    INITIALIZE
        state.numberOfEchoCalls = 0;
        state.numberOfBurnCalls = 0;
        state.active = 0;
    _

    inline static void getCurrentDate(const QPI::QpiContextProcedureCall& qpi, uint32& res)
    {
        res = ((qpi.year() - 24) << 26) | (qpi.month() << 22) | (qpi.day() << 17) | (qpi.hour() << 12) | (qpi.minute() << 6) | qpi.second();
    }
};
