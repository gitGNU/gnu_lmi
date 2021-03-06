// Basic values.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// http://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile.hpp"

#include "basic_values.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "calendar_date.hpp"
#include "contains.hpp"
#include "data_directory.hpp"
#include "database.hpp"
#include "death_benefits.hpp"
#include "et_vector.hpp"
#include "fund_data.hpp"
#include "global_settings.hpp"
#include "gpt_specamt.hpp"
#include "ieee754.hpp"                  // ldbl_eps_plus_one()
#include "ihs_irc7702.hpp"
#include "ihs_irc7702a.hpp"
#include "input.hpp"
#include "interest_rates.hpp"
#include "loads.hpp"
#include "math_functors.hpp"
#include "mc_enum_types_aux.hpp"        // mc_str()
#include "mortality_rates.hpp"
#include "outlay.hpp"
#include "premium_tax.hpp"
#include "product_data.hpp"
#include "rounding_rules.hpp"
#include "stratified_charges.hpp"
#include "surrchg_rates.hpp"
#include "value_cast.hpp"

#include <algorithm>
#include <cmath>                        // std::pow()
#include <limits>
#include <numeric>
#include <stdexcept>

//============================================================================
BasicValues::BasicValues(Input const& input)
    :yare_input_         (input)
    ,DefnLifeIns_        (mce_cvat)
    ,DefnMaterialChange_ (mce_unnecessary_premium)
    ,Equiv7702DBO3       (mce_option1_for_7702)
    ,MaxWDDed_           (mce_twelve_times_last)
    ,MaxLoanDed_         (mce_twelve_times_last)
    ,StateOfJurisdiction_(mce_s_CT)
    ,StateOfDomicile_    (mce_s_CT)
    ,PremiumTaxState_    (mce_s_CT)
{
    Init();
}

//============================================================================
// TODO ?? Not for general use--use for GPT server only. This is bad design. TAXATION !! Eliminate this.
BasicValues::BasicValues
    (std::string  const& a_ProductName
    ,mcenum_gender       a_Gender
    ,mcenum_class        a_UnderwritingClass
    ,mcenum_smoking      a_Smoker
    ,int                 a_IssueAge
    ,mcenum_uw_basis     a_UnderwritingBasis
    ,mcenum_state        a_StateOfJurisdiction
    ,double              a_FaceAmount
    ,mcenum_dbopt_7702   a_DBOptFor7702
    ,bool                a_AdbInForce
    ,double              a_TargetPremium
    // TODO ?? Need loan rate type here?
    )
    :yare_input_         (Input())
    ,DefnLifeIns_        (mce_cvat)
    ,DefnMaterialChange_ (mce_unnecessary_premium)
    ,Equiv7702DBO3       (a_DBOptFor7702)
    ,MaxWDDed_           (mce_twelve_times_last)
    ,MaxLoanDed_         (mce_twelve_times_last)
    ,StateOfJurisdiction_(a_StateOfJurisdiction)
    ,StateOfDomicile_    (a_StateOfJurisdiction)
    ,PremiumTaxState_    (a_StateOfJurisdiction)
    ,InitialTargetPremium(a_TargetPremium)
{
    yare_input_.IssueAge                   = a_IssueAge           ;
    yare_input_.RetirementAge              = a_IssueAge           ;
    yare_input_.Gender                     = a_Gender             ;
    yare_input_.Smoking                    = a_Smoker             ;
    yare_input_.UnderwritingClass          = a_UnderwritingClass  ;
    if(a_AdbInForce)
        {
        yare_input_.AccidentalDeathBenefit     = mce_yes          ;
        }
    else
        {
        yare_input_.AccidentalDeathBenefit     = mce_no           ;
        }
    yare_input_.GroupUnderwritingType      = a_UnderwritingBasis  ;
    yare_input_.ProductName                = a_ProductName        ;
    yare_input_.PremiumTaxState            = a_StateOfJurisdiction;
    yare_input_.DefinitionOfLifeInsurance  = mce_gpt              ;
    yare_input_.DefinitionOfMaterialChange = mce_adjustment_event ;

    yare_input_.SpecifiedAmount            .assign(1, a_FaceAmount);

    mce_dbopt const z
        (mce_option1_for_7702 == a_DBOptFor7702 ? mce_option1
        :mce_option2_for_7702 == a_DBOptFor7702 ? mce_option2
        :throw std::runtime_error("Unexpected DB option.")
        );
    yare_input_.DeathBenefitOption         .assign(1, z.value());

    GPTServerInit();
}

//============================================================================
void BasicValues::Init()
{
    ProductData_.reset(new product_data(yare_input_.ProductName));
    Database_.reset(new product_database(yare_input_));

    SetPermanentInvariants();

    StateOfDomicile_ = mc_state_from_string(ProductData_->datum("InsCoDomicile"));
    StateOfJurisdiction_ = yare_input_.StateOfJurisdiction;
    PremiumTaxState_     = yare_input_.PremiumTaxState    ;

    if
        (   !Database_->Query(DB_StateApproved)
        &&  !global_settings::instance().ash_nazg()
        &&  !global_settings::instance().regression_testing()
        )
        {
        alarum()
            << "Product "
            << yare_input_.ProductName
            << " not approved in state "
            << mc_str(GetStateOfJurisdiction())
            << "."
            << LMI_FLUSH
            ;
        }

    IssueAge = yare_input_.IssueAge;
    RetAge   = yare_input_.RetirementAge;
    LMI_ASSERT(IssueAge < 100);
    LMI_ASSERT(RetAge <= 100);
    LMI_ASSERT(yare_input_.RetireesCanEnroll || IssueAge <= RetAge);

    EndtAge = static_cast<int>(Database_->Query(DB_MaturityAge));
    Length = EndtAge - IssueAge;

    ledger_type_ =
        static_cast<mcenum_ledger_type>
            (static_cast<int>
                (Database_->Query(DB_LedgerType))
            )
        ;
    nonillustrated_       = static_cast<bool>(Database_->Query(DB_Nonillustrated));
    bool no_longer_issued = static_cast<bool>(Database_->Query(DB_NoLongerIssued));
    bool is_new_business  = yare_input_.EffectiveDate == yare_input_.InforceAsOfDate;
    no_can_issue_         = no_longer_issued && is_new_business;
    IsSubjectToIllustrationReg_ = is_subject_to_ill_reg(ledger_type());

    if(IssueAge < Database_->Query(DB_MinIssAge))
        {
        alarum()
            << "Issue age "
            << IssueAge
            << " less than minimum "
            << Database_->Query(DB_MinIssAge)
            << '.'
            << LMI_FLUSH
            ;
        }
    if(Database_->Query(DB_MaxIssAge) < IssueAge)
        {
        alarum()
            << "Issue age "
            << IssueAge
            << " greater than maximum "
            << Database_->Query(DB_MaxIssAge)
            << '.'
            << LMI_FLUSH
            ;
        }
    FundData_.reset(new FundData(AddDataDir(ProductData_->datum("FundFilename"))));
    RoundingRules_.reset
        (new rounding_rules(AddDataDir(ProductData_->datum("RoundingFilename")))
        );
    SetRoundingFunctors();
    StratifiedCharges_.reset
        (new stratified_charges(AddDataDir(ProductData_->datum("TierFilename")))
        );
    SpreadFor7702_.assign
        (Length
        ,StratifiedCharges_->minimum_tiered_spread_for_7702()
        );

    // Multilife contracts will need a vector of mortality-rate objects.

    // Mortality and interest rates require database and rounding.
    // Interest rates require tiered data and 7702 spread.
    MortalityRates_.reset(new MortalityRates (*this));
    InterestRates_ .reset(new InterestRates  (*this));
    // Surrender-charge rates will eventually require mortality rates.
    SurrChgRates_  .reset(new SurrChgRates   (*Database_));
    DeathBfts_     .reset(new death_benefits (GetLength(), yare_input_));
    // Outlay requires only input; it might someday use interest rates.
    Outlay_        .reset(new modal_outlay   (yare_input_));
    PremiumTax_    .reset(new premium_tax    (PremiumTaxState_, StateOfDomicile_, yare_input_.AmortizePremiumLoad, *Database_, *StratifiedCharges_));
    Loads_         .reset(new Loads          (*this));

    // The target premium can't be ascertained yet if specamt is
    // determined by a strategy. This data member is used only by
    // Init7702(), and is meaningful only when that function is called
    // by GPTServerInit(); the value assigned here is overridden by a
    // downstream call to Irc7702::Initialize7702(). TAXATION !! So
    // eliminate the member when it becomes unnecessary.
    InitialTargetPremium = 0.0;

    SetMaxSurvivalDur();

    Init7702();
    Init7702A();
}

//============================================================================
#include "ihs_x_type.hpp"               // x_product_rule_violated TAXATION !! remove later
// TODO ??  Not for general use--use for GPT server only, for now. TAXATION !! refactor later
void BasicValues::GPTServerInit()
{
    ProductData_.reset(new product_data(yare_input_.ProductName));
    Database_.reset(new product_database(yare_input_));

    SetPermanentInvariants();

    IssueAge = yare_input_.IssueAge;
    RetAge   = yare_input_.RetirementAge;
    LMI_ASSERT(IssueAge < 100);
    LMI_ASSERT(RetAge <= 100);
    LMI_ASSERT(yare_input_.RetireesCanEnroll || IssueAge <= RetAge);

    StateOfDomicile_ = mc_state_from_string(ProductData_->datum("InsCoDomicile"));
    StateOfJurisdiction_ = yare_input_.StateOfJurisdiction;
    PremiumTaxState_     = yare_input_.PremiumTaxState    ;

    EndtAge = static_cast<int>(Database_->Query(DB_MaturityAge));
    Length = EndtAge - IssueAge;

    yare_input_.ExtraMonthlyCustodialFee  .resize(Length);
    yare_input_.ExtraCompensationOnAssets .resize(Length);
    yare_input_.ExtraCompensationOnPremium.resize(Length);
    yare_input_.CurrentCoiMultiplier      .assign(Length, 1.0);
    yare_input_.SpecifiedAmount           .assign(Length, yare_input_.SpecifiedAmount   [0]);
    yare_input_.DeathBenefitOption        .assign(Length, yare_input_.DeathBenefitOption[0]);
    yare_input_.FlatExtra                 .resize(Length);

    ledger_type_ =
        static_cast<mcenum_ledger_type>
            (static_cast<int>
                (Database_->Query(DB_LedgerType))
            )
        ;
    nonillustrated_       = static_cast<bool>(Database_->Query(DB_Nonillustrated));
    bool no_longer_issued = static_cast<bool>(Database_->Query(DB_NoLongerIssued));
    bool is_new_business  = yare_input_.EffectiveDate == yare_input_.InforceAsOfDate;
    no_can_issue_         = no_longer_issued && is_new_business;
    IsSubjectToIllustrationReg_ = is_subject_to_ill_reg(ledger_type());

    if(IssueAge < Database_->Query(DB_MinIssAge))
        {
        throw x_product_rule_violated
            (
            std::string("Issue age less than minimum")
            );
        }
    if(Database_->Query(DB_MaxIssAge) < IssueAge)
        {
        throw x_product_rule_violated
            (
            std::string("Issue age greater than maximum")
            );
        }
//  FundData_       = new FundData
//      (AddDataDir(ProductData_->datum("FundFilename"))
//      );
    RoundingRules_.reset
        (new rounding_rules(AddDataDir(ProductData_->datum("RoundingFilename")))
        );
    SetRoundingFunctors();
    StratifiedCharges_.reset
        (new stratified_charges(AddDataDir(ProductData_->datum("TierFilename")))
        );
    SpreadFor7702_.assign
        (Length
        ,StratifiedCharges_->minimum_tiered_spread_for_7702()
        );

    // Requires database.
    MortalityRates_.reset(new MortalityRates (*this)); // Used by certain target-premium calculations.
//  InterestRates_ .reset(new InterestRates  (*this));
    // Will require mortality rates eventually.
//  SurrChgRates_  .reset(new SurrChgRates   (Database_));
//  DeathBfts_     .reset(new death_benefits (GetLength(), yare_input_));
    // Outlay requires only input; it might someday use interest rates.
//  Outlay_        .reset(new modal_outlay   (yare_input_));
    PremiumTax_    .reset(new premium_tax    (PremiumTaxState_, StateOfDomicile_, yare_input_.AmortizePremiumLoad, *Database_, *StratifiedCharges_));
    Loads_         .reset(new Loads          (*this));

    SetMaxSurvivalDur();

    Init7702();
}

//============================================================================
// TODO ?? Does this belong in the funds class? Consider merging it
// with code in AccountValue::SetInitialValues().
double BasicValues::InvestmentManagementFee() const
{
    if(!Database_->Query(DB_AllowSepAcct))
        {
        return 0.0;
        }

    if(yare_input_.OverrideFundManagementFee)
        {
        return yare_input_.InputFundManagementFee / 10000.0L;
        }

    double z = 0.0;
    double total_sepacct_allocations = 0.0;
    FundData const& Funds = *FundData_;

    for(int j = 0; j < Funds.GetNumberOfFunds(); j++)
        {
        double weight;
        // If average of all funds, then use equal weights, but
        // disregard "custom" funds--that is, set their weights to
        // zero. Custom funds are those whose name begins with "Custom".
        // Reason: "average" means average of the normally-available
        // funds only. Use the average not only if explicitly chosen
        // in input, but also if all payments are allocated to the
        // general account: in the latter case, some separate-account
        // fee or rate may nonetheless be wanted on output, and the
        // arithmetic mean is more reasonable than zero.
        if(yare_input_.UseAverageOfAllFunds || 0.0 == premium_allocation_to_sepacct(yare_input_))
            {
            bool ignore = 0 == Funds.GetFundInfo(j).ShortName().find("custom");
            weight = ignore ? 0.0 : 1.0;
            }
        // If fund mgmt fee not overridden by average of all funds,
        // then use input weights.
/*
        else
            {
            // Allow hardcoded number of funds < Funds.GetNumberOfFunds
            // so an accurate fund average can be calculated (based
            // on the funds in the '.funds' file), even though the inputs
            // class may not accommodate that many funds. If j falls
            // outside the range of yare_input_.FundAllocations, use a
            // weight of zero.
            bool legal_storage = j < yare_input_.FundAllocations.size();
            double fund_alloc = yare_input_.FundAllocations[j];
            weight = legal_storage ? fund_alloc : 0.0;
            }
*/
        else if(j < static_cast<int>(yare_input_.FundAllocations.size()))
            {
            // TODO ?? Can this be correct? Shouldn't we be looking to
            // the '.funds' file rather than the input class?
            //
            // Allow hardcoded number of funds < Funds.GetNumberOfFunds
            // so an accurate fund average can be calculated (based
            // on the funds in the '.funds' file), even though the inputs
            // class may not accommodate that many funds. If j falls
            // outside the range of yare_input_.FundAllocations, use a
            // weight of zero.
            weight = yare_input_.FundAllocations[j];
            }
        else
            {
            weight = 0.0;
            }

        if(0.0 != weight)
            {
            z += weight * Funds.GetFundInfo(j).ScalarIMF();
            total_sepacct_allocations += weight;
            }
        }

    // Spread over separate account funds only
    if(0.0 != total_sepacct_allocations)
        {
        // Convert from basis points
        z /= 10000.0 * total_sepacct_allocations;
        }

    return z;
}

// TAXATION !! Reconsider unconditional initialization.
// TAXATION !! For 7702A, offer a choice: tables or first principles.

/// Initialize 7702 object.
///
/// This function is called unconditionally, even for CVAT cases that
/// read CVAT corridor factors from a table, for two reasons:
///   - GLP and GSP premium and specamt strategies are always offered;
///   - at least one known product uses GLP as a handy proxy for a
///     minimum no-lapse premium, even when the GPT is not elected.
/// TAXATION !! OTOH, such strategies need not always be offered, and
/// the cited product was simplified to use a table lookup.
///
/// To conform to the practices of certain admin systems, DCV COI
/// rates are stored in a rounded table, but calculations from first
/// principles (GLP, GSP, 7PP, e.g.) use unrounded monthly rates;
/// thus, necessary premium uses both. But this is immaterial.
/// TAXATION !! DATABASE !! The database should offer rounding
/// options; and should this comment be moved to the TUs that
/// implement taxation?

void BasicValues::Init7702()
{
    Mly7702qc = GetIrc7702QRates();
    double max_coi_rate = Database_->Query(DB_MaxMonthlyCoiRate);
    LMI_ASSERT(0.0 != max_coi_rate);
    max_coi_rate = 1.0 / max_coi_rate;
    assign(Mly7702qc, apply_binary(coi_rate_from_q<double>(), Mly7702qc, max_coi_rate));

    MlyDcvqc = Mly7702qc;
    std::transform
        (MlyDcvqc.begin()
        ,MlyDcvqc.end()
        ,MlyDcvqc.begin()
        ,round_coi_rate()
        );

    // Monthly guar net int for 7702, with 4 or 6% min, is
    //   greater of {4%, 6%} and annual guar int rate
    //   less 7702 spread
    // TODO ?? TAXATION !! We need to subtract other things too, e.g. comp (sometimes)...
    //   transformed to monthly (simple subtraction?).
    // These interest rates belong here because they're used by
    // DCV calculations in the account value class as well as
    // GPT calculations in the 7702 class.

    std::vector<double> guar_int;
    Database_->Query(guar_int, DB_GuarInt);
// TAXATION !! Rework this. The intention is to make the 7702 interest
// rate no less, at any duration, than the guaranteed loan rate--here,
// the fixed rate charged on loans, minus the guaranteed loan spread
// (if any).
/*
    switch(yare_input_.LoanRateType)
        {
        case mce_fixed_loan_rate:
            {
            std::vector<double> gross_loan_rate;
            Database_->Query(gross_loan_rate, DB_FixedLoanRate);
            std::vector<double> guar_loan_spread;
            Database_->Query(guar_loan_spread, DB_GuarRegLoanSpread);
            // ET !! std::vector<double> guar_loan_rate = gross_loan_rate - guar_loan_spread;
            std::vector<double> guar_loan_rate(Length);
            std::transform
                (gross_loan_rate.begin()
                ,gross_loan_rate.end()
                ,guar_loan_spread.begin()
                ,guar_loan_rate.begin()
                ,std::minus<double>()
                );
            // ET !! guar_int = max(guar_int, guar_loan_spread);
            // TODO ?? But that looks incorrect when written clearly!
            // Perhaps this old comment:
            //   APL: guar_int gets guar_int max gross_loan_rate - guar_loan_spread
            // suggests the actual intention.
            std::transform
                (guar_int.begin()
                ,guar_int.end()
                ,guar_loan_spread.begin()
                ,guar_int.begin()
                ,greater_of<double>()
                );
            }
            break;
        case mce_variable_loan_rate:
            {
            // do nothing
            }
            break;
        default:
            {
            alarum()
                << "Case "
                << yare_input_.LoanRateType
                << " not found."
                << LMI_FLUSH
                ;
            }
        }
*/

    Mly7702iGlp.assign(Length, 0.0);
    assign
        (Mly7702iGlp
        ,apply_unary
            (i_upper_12_over_12_from_i<double>()
            ,apply_binary(greater_of<double>(), 0.04, guar_int) - SpreadFor7702_
            )
        );

    Mly7702iGsp.assign(Length, 0.0);
    assign
        (Mly7702iGsp
        ,apply_unary
            (i_upper_12_over_12_from_i<double>()
            ,apply_binary(greater_of<double>(), 0.06, guar_int) - SpreadFor7702_
            )
        );

    Database_->Query(Mly7702ig, DB_NaarDiscount);

    // TODO ?? We should avoid reading the rate file again; but
    // the GPT server doesn't initialize a MortalityRates object
    // that would hold those rates. TAXATION !! Rework this.
    std::vector<double> local_mly_charge_add(Length, 0.0);
    if(yare_input_.AccidentalDeathBenefit)
        {
        local_mly_charge_add = GetAdbRates();
        }

    double local_adb_limit = Database_->Query(DB_AdbIsQAB) ? AdbLimit : 0.0;

    Irc7702_.reset
        (new Irc7702
            (yare_input_.DefinitionOfLifeInsurance
            ,yare_input_.IssueAge
            ,EndtAge
            ,Mly7702qc
            ,Mly7702iGlp
            ,Mly7702iGsp
            ,Mly7702ig
            ,SpreadFor7702_
            ,yare_input_.SpecifiedAmount[0] + yare_input_.TermRiderAmount
            ,yare_input_.SpecifiedAmount[0] + yare_input_.TermRiderAmount
            ,yare_input_.SpecifiedAmount[0] + yare_input_.TermRiderAmount
            ,effective_dbopt_7702(yare_input_.DeathBenefitOption[0], Equiv7702DBO3)
            ,Loads_->annual_policy_fee    (mce_gen_curr)
            ,Loads_->monthly_policy_fee   (mce_gen_curr)
            ,Loads_->specified_amount_load(mce_gen_curr)
            ,SpecAmtLoadLimit
            ,local_mly_charge_add
            ,local_adb_limit
/// TAXATION !! No contemporary authority seems to believe that a
/// change in the premium-tax rate, even if passed through to the
/// policyowner, is a 7702A material change or a GPT adjustment event.
/// These loads should instead reflect the lowest premium-tax rate.
            ,Loads_->target_premium_load_excluding_premium_tax()
            ,Loads_->excess_premium_load_excluding_premium_tax()
            ,InitialTargetPremium
            ,round_min_premium()
            ,round_max_premium()
            ,round_min_specamt()
            ,round_max_specamt()
            ,yare_input_.InforceYear
            ,yare_input_.InforceMonth
            ,yare_input_.InforceGlp
            ,yare_input_.InforceCumulativeGlp
            ,yare_input_.InforceGsp
            ,yare_input_.InforceCumulativeGptPremiumsPaid
            )
        );
}

//============================================================================
void BasicValues::Init7702A()
{
    Irc7702A_.reset
        (new Irc7702A
            (DefnLifeIns_
            ,DefnMaterialChange_
            ,false // TODO ?? TAXATION !! Joint life: hardcoded for now.
            ,yare_input_.AvoidMecMethod
            ,true  // TODO ?? TAXATION !! Use table for 7pp: hardcoded for now.
            ,true  // TODO ?? TAXATION !! Use table for NSP: hardcoded for now.
            ,MortalityRates_->SevenPayRates()
            ,MortalityRates_->CvatNspRates()
            ,round_max_premium()
            )
        );
}

/// Public function used for GPT specamt calculation.

double BasicValues::GetAnnualTgtPrem(int a_year, double a_specamt) const
{
    return GetModalTgtPrem(a_year, mce_annual, a_specamt);
}

/// Establish up front some values that cannot later change.
///
/// Values set here depend on Database_, and thus on yare_input_ and
/// on ProductData_, but not on any other shared_ptr members--so they
/// can reliably be used in initializing those other members.

void BasicValues::SetPermanentInvariants()
{
    MinIssSpecAmt       = Database_->Query(DB_MinIssSpecAmt        );
    MinIssBaseSpecAmt   = Database_->Query(DB_MinIssBaseSpecAmt    );
    MinRenlSpecAmt      = Database_->Query(DB_MinRenlSpecAmt       );
    MinRenlBaseSpecAmt  = Database_->Query(DB_MinRenlBaseSpecAmt   );
    NoLapseOpt1Only     = Database_->Query(DB_NoLapseDbo1Only      );
    NoLapseUnratedOnly  = Database_->Query(DB_NoLapseUnratedOnly   );
    OptChgCanIncrSA     = Database_->Query(DB_DboChgCanIncrSpecAmt );
    OptChgCanDecrSA     = Database_->Query(DB_DboChgCanDecrSpecAmt );
    WDCanDecrSADBO1     = Database_->Query(DB_WdCanDecrSpecAmtDbo1 );
    WDCanDecrSADBO2     = Database_->Query(DB_WdCanDecrSpecAmtDbo2 );
    WDCanDecrSADBO3     = Database_->Query(DB_WdCanDecrSpecAmtDbo3 );
    MaxIncrAge          = static_cast<int>(Database_->Query(DB_MaxIncrAge));
    WaivePmTxInt1035    = Database_->Query(DB_WaivePremTaxInt1035  );
    TermIsNotRider      = Database_->Query(DB_TermIsNotRider       );
    TermForcedConvAge   = static_cast<int>(Database_->Query(DB_TermForcedConvAge));
    TermForcedConvDur   = static_cast<int>(Database_->Query(DB_TermForcedConvDur));
    ExpPerKLimit        = Database_->Query(DB_ExpSpecAmtLimit      );
    MinPremType         = static_cast<oenum_modal_prem_type>(static_cast<int>(Database_->Query(DB_MinPremType)));
    TgtPremType         = static_cast<oenum_modal_prem_type>(static_cast<int>(Database_->Query(DB_TgtPremType)));
    TgtPremFixedAtIssue = Database_->Query(DB_TgtPremFixedAtIssue  );
    TgtPremMonthlyPolFee= Database_->Query(DB_TgtPremMonthlyPolFee );
    // Assertion: see comments on GetModalPremTgtFromTable().
    LMI_ASSERT
        (  0.0 == TgtPremMonthlyPolFee
        || (oe_modal_table == TgtPremType && oe_modal_table != MinPremType)
        );
    CurrCoiTable0Limit  = Database_->Query(DB_CurrCoiTable0Limit   );
    CurrCoiTable1Limit  = Database_->Query(DB_CurrCoiTable1Limit   );
    LMI_ASSERT(0.0                <= CurrCoiTable0Limit);
    LMI_ASSERT(CurrCoiTable0Limit <= CurrCoiTable1Limit);
    CoiInforceReentry   = static_cast<e_actuarial_table_method>(static_cast<int>(Database_->Query(DB_CoiInforceReentry)));
    MaxWDDed_           = static_cast<mcenum_anticipated_deduction>(static_cast<int>(Database_->Query(DB_MaxWdDed)));
    MaxWdGenAcctValMult = Database_->Query(DB_MaxWdGenAcctValMult  );
    MaxWdSepAcctValMult = Database_->Query(DB_MaxWdSepAcctValMult  );
    AllowPrefLoan       = static_cast<bool>(Database_->Query(DB_AllowPrefLoan));
    MaxLoanDed_         = static_cast<mcenum_anticipated_deduction>(static_cast<int>(Database_->Query(DB_MaxLoanDed)));
    MaxLoanAVMult       = Database_->Query(DB_MaxLoanAcctValMult   );
    FirstPrefLoanYear   = static_cast<int>(Database_->Query(DB_FirstPrefLoanYear));
    NoLapseMinDur       = static_cast<int>(Database_->Query(DB_NoLapseMinDur));
    NoLapseMinAge       = static_cast<int>(Database_->Query(DB_NoLapseMinAge));
    AdbLimit            = Database_->Query(DB_AdbLimit             );
    WpLimit             = Database_->Query(DB_WpLimit              );
    SpecAmtLoadLimit    = Database_->Query(DB_SpecAmtLoadLimit     );
    MinWD               = Database_->Query(DB_MinWd                );
    WDFee               = Database_->Query(DB_WdFee                );
    WDFeeRate           = Database_->Query(DB_WdFeeRate            );
    AllowChangeToDBO2   = Database_->Query(DB_AllowChangeToDbo2    );
    AllowSAIncr         = Database_->Query(DB_AllowSpecAmtIncr     );
    NoLapseAlwaysActive = Database_->Query(DB_NoLapseAlwaysActive  );
    WaiverChargeMethod  = static_cast<oenum_waiver_charge_method>(static_cast<int>(Database_->Query(DB_WpChargeMethod)));
    LapseIgnoresSurrChg = Database_->Query(DB_LapseIgnoresSurrChg  );
    SurrChgOnIncr       = Database_->Query(DB_SurrChgOnIncr        );
    SurrChgOnDecr       = Database_->Query(DB_SurrChgOnDecr        );
    LMI_ASSERT(!SurrChgOnDecr); // Surrchg change on decrease not supported.

    Database_->Query(FreeWDProportion, DB_FreeWdProportion);

    Database_->Query(DBDiscountRate, DB_NaarDiscount);
    LMI_ASSERT(!contains(DBDiscountRate, -1.0));
// This would be more natural:
//    assign(DBDiscountRate, 1.0 / (1.0 + DBDiscountRate));
// but we avoid it for the nonce because it causes slight regression errors.
    assign(DBDiscountRate, 1.0 + DBDiscountRate);
    assign(DBDiscountRate, 1.0 / DBDiscountRate);

    CalculateComp       = Database_->Query(DB_CalculateComp        );
    Database_->Query(AssetComp , DB_AssetComp);
    Database_->Query(CompTarget, DB_CompTarget);
    Database_->Query(CompExcess, DB_CompExcess);

    MandEIsDynamic      = Database_->Query(DB_DynamicMandE         );
    SepAcctLoadIsDynamic= Database_->Query(DB_DynamicSepAcctLoad   );

    UseUnusualCOIBanding= Database_->Query(DB_UnusualCoiBanding    );

    // 'Unusual' COI banding accommodates a particular idiosyncratic
    // product which has no term rider and doesn't permit experience
    // rating, so we assert those preconditions and write simple code
    // for 'unusual' COI banding that ignores those features.
    LMI_ASSERT(!(UseUnusualCOIBanding && yare_input_.UseExperienceRating));
    LMI_ASSERT(!(UseUnusualCOIBanding && Database_->Query(DB_AllowTerm)));

    // Flat extras can be used even with guaranteed issue, e.g., for
    // aviation, occupation, avocation, or foreign travel. Admin
    // systems typically don't distinguish these from medical flats,
    // so neither does lmi--that information wouldn't be available for
    // inforce illustrations.
    //
    // However, table ratings may be restricted, e.g., to medically-
    // underwritten contracts, by setting 'DB_AllowSubstdTable' in a
    // product's database. See the example in 'dbdict.cpp', which
    // varies by yare_input_.GroupUnderwritingType; no restriction is
    // expressly coded here in terms of that field because it is a
    // database axis across which 'DB_AllowSubstdTable' already can
    // vary. Even a rule as apparently reasonable as forbidding table
    // ratings with simplified issue would thus be out of place here:
    // the database can express such a rule handily, and at least one
    // real-world product is known not to follow it. It is important
    // to put aside prior notions of what SI and GI might connote, and
    // realize that for lmi underwriting class is predominantly a
    // database-lookup axis.
    //
    // Table ratings are used only with COI rates for the "Rated"
    // class, so that table multipliers can be applied to a set of COI
    // rates distinct from standard (as is the practice of at least
    // one company). If no such distinction is wanted, then "Rated"
    // and "Standard" can simply point to the same rate table (as
    // happens by default if rates don't vary by underwriting class).
    // The additional "Rated" class induces an extra "Rated" choice in
    // the GUI, which must be selected to enable table ratings; this
    // may be seen as a surprising complication, or as a useful safety
    // feature, but either way no end user has ever objected to it.
    if
        (   mce_table_none != yare_input_.SubstandardTable
        &&  !(Database_->Query(DB_AllowSubstdTable) && mce_rated == yare_input_.UnderwritingClass)
        )
        {
        alarum() << "Substandard table ratings not permitted." << LMI_FLUSH;
        }

    // SOMEDAY !! WP and ADB shouldn't always be forbidden with table
    // ratings and flat extras. For now, they're not supported due to
    // lack of demand and complexity. These riders are likely to
    // require their own ratings that differ from the base policy's
    // because the insured contingencies differ.
    //
    // Spouse and child riders are not similarly restricted because
    // their rates don't depend on the main insured's health, and the
    // people they cover are unlikely to be underwritten.
    if(is_policy_rated(yare_input_) && yare_input_.WaiverOfPremiumBenefit)
        {
        alarum() << "Substandard waiver of premium not supported." << LMI_FLUSH;
        }
    if(is_policy_rated(yare_input_) && yare_input_.AccidentalDeathBenefit)
        {
        alarum()
            << "Substandard accidental death rider not supported."
            << LMI_FLUSH
            ;
        }

    DefnLifeIns_        = yare_input_.DefinitionOfLifeInsurance;
    DefnMaterialChange_ = yare_input_.DefinitionOfMaterialChange;
    // TAXATION !! For the nonce, input 'DefinitionOfMaterialChange'
    // is ignored without the most-privileged password. In the future,
    // the input class will distinguish CVAT from GPT material-change
    // definitions, and 'DefinitionOfMaterialChange' will be removed.
    if(!global_settings::instance().ash_nazg())
        {
        mcenum_defn_material_change const z = static_cast<mcenum_defn_material_change>(static_cast<int>(Database_->Query(DB_CvatMatChangeDefn)));
        DefnMaterialChange_ = (mce_gpt == DefnLifeIns_) ? mce_adjustment_event : z;
        }
    Equiv7702DBO3       = static_cast<mcenum_dbopt_7702>(static_cast<int>(Database_->Query(DB_Equiv7702Dbo3)));
    TermIsDbFor7702     = 1.0 == Database_->Query(DB_TermIsQABOrDb7702 );
    TermIsDbFor7702A    = 1.0 == Database_->Query(DB_TermIsQABOrDb7702A);
    MaxNAAR             = yare_input_.MaximumNaar;

    Database_->Query(MinPremIntSpread_, DB_MinPremIntSpread);
}

namespace
{
void set_rounding_rule(round_to<double>& functor, rounding_parameters const& z)
{
    functor = round_to<double>(z.decimals(), z.style().value());
}
} // Unnamed namespace.

void BasicValues::SetRoundingFunctors()
{
    set_rounding_rule(round_specamt_           , RoundingRules_->datum("RoundSpecAmt"     ));
    set_rounding_rule(round_death_benefit_     , RoundingRules_->datum("RoundDeathBft"    ));
    set_rounding_rule(round_naar_              , RoundingRules_->datum("RoundNaar"        ));
    set_rounding_rule(round_coi_rate_          , RoundingRules_->datum("RoundCoiRate"     ));
    set_rounding_rule(round_coi_charge_        , RoundingRules_->datum("RoundCoiCharge"   ));
    set_rounding_rule(round_gross_premium_     , RoundingRules_->datum("RoundGrossPrem"   ));
    set_rounding_rule(round_net_premium_       , RoundingRules_->datum("RoundNetPrem"     ));
    set_rounding_rule(round_interest_rate_     , RoundingRules_->datum("RoundIntRate"     ));
    set_rounding_rule(round_interest_credit_   , RoundingRules_->datum("RoundIntCredit"   ));
    set_rounding_rule(round_withdrawal_        , RoundingRules_->datum("RoundWithdrawal"  ));
    set_rounding_rule(round_loan_              , RoundingRules_->datum("RoundLoan"        ));
    set_rounding_rule(round_interest_rate_7702_, RoundingRules_->datum("RoundIntRate7702" ));
    set_rounding_rule(round_corridor_factor_   , RoundingRules_->datum("RoundCorrFactor"  ));
    set_rounding_rule(round_nsp_rate_7702_     , RoundingRules_->datum("RoundNspRate7702" ));
    set_rounding_rule(round_seven_pay_rate_    , RoundingRules_->datum("RoundSevenPayRate"));
    set_rounding_rule(round_surrender_charge_  , RoundingRules_->datum("RoundSurrCharge"  ));
    set_rounding_rule(round_irr_               , RoundingRules_->datum("RoundIrr"         ));
    set_rounding_rule(round_min_specamt_       , RoundingRules_->datum("RoundMinSpecamt"  ));
    set_rounding_rule(round_max_specamt_       , RoundingRules_->datum("RoundMaxSpecamt"  ));
    set_rounding_rule(round_min_premium_       , RoundingRules_->datum("RoundMinPrem"     ));
    set_rounding_rule(round_max_premium_       , RoundingRules_->datum("RoundMaxPrem"     ));
    set_rounding_rule(round_min_init_premium_  , RoundingRules_->datum("RoundMinInitPrem" ));
}

/// Establish maximum survivorship duration.
///
/// Depends on MortalityRates_ for life-expectancy calculation.

void BasicValues::SetMaxSurvivalDur()
{
    switch(yare_input_.SurviveToType)
        {
        case mce_no_survival_limit:
            {
            MaxSurvivalDur = EndtAge;
            }
            break;
        case mce_survive_to_age:
            {
            MaxSurvivalDur = yare_input_.SurviveToAge - yare_input_.IssueAge;
            }
            break;
        case mce_survive_to_year:
            {
            MaxSurvivalDur = yare_input_.SurviveToYear;
            }
            break;
        case mce_survive_to_expectancy:
            {
            std::vector<double> z(Length);
            assign(z, 1.0 - MortalityRates_->PartialMortalityQ());
            // ET !! In APL, this would be [writing multiplication as '*']
            //   +/*\1-z
            // It would be nice to have a concise representation for that.
            std::partial_sum(z.begin(), z.end(), z.begin(), std::multiplies<double>());
            MaxSurvivalDur = std::accumulate(z.begin(), z.end(), 0.0);
            }
            break;
        default:
            {
            alarum()
                << "Case "
                << yare_input_.SurviveToType
                << " not found."
                << LMI_FLUSH
                ;
            }
        }
    LMI_ASSERT(MaxSurvivalDur <= EndtAge);
}

//============================================================================
double BasicValues::GetModalMinPrem
    (int         a_year
    ,mcenum_mode a_mode
    ,double      a_specamt
    ) const
{
    return GetModalPrem(a_year, a_mode, a_specamt, MinPremType);
}

/// Calculate target premium.
///
/// 'TgtPremMonthlyPolFee' is not added here, because it is added in
/// GetModalPremTgtFromTable().

double BasicValues::GetModalTgtPrem
    (int         a_year
    ,mcenum_mode a_mode
    ,double      a_specamt
    ) const
{
    int const target_year = TgtPremFixedAtIssue ? 0 : a_year;
    return GetModalPrem(target_year, a_mode, a_specamt, TgtPremType);
}

//============================================================================
double BasicValues::GetModalPrem
    (int                   a_year
    ,mcenum_mode           a_mode
    ,double                a_specamt
    ,oenum_modal_prem_type a_prem_type
    ) const
{
    if(oe_monthly_deduction == a_prem_type)
        {
        return GetModalPremMlyDed      (a_year, a_mode, a_specamt);
        }
    else if(oe_modal_nonmec == a_prem_type)
        {
        return GetModalPremMaxNonMec   (a_year, a_mode, a_specamt);
        }
    else if(oe_modal_table == a_prem_type)
        {
        return GetModalPremTgtFromTable(a_year, a_mode, a_specamt);
        }
    else
        {
        alarum()
            << "Unknown modal premium type " << a_prem_type << '.'
            << LMI_FLUSH
            ;
        }
    return 0.0;
}

/// Calculate premium using a seven-pay ratio.
///
/// Only the initial seven-pay premium rate is used here. Material
/// changes dramatically complicate the relationship between premium
/// and specified amount. Thus, arguments should represent initial
/// premium and mode.

double BasicValues::GetModalPremMaxNonMec
    (int      // a_year // Unused.
    ,mcenum_mode a_mode
    ,double      a_specamt
    ) const
{
    // TAXATION !! No table available if 7PP calculated from first principles.
    double temp = MortalityRates_->SevenPayRates()[0];
    return round_max_premium()(temp * ldbl_eps_plus_one() * a_specamt / a_mode);
}

/// Calculate premium using a target-premium ratio.
///
/// Only the initial target-premium rate is used here, because that's
/// generally fixed at issue. This calculation remains naive in that
/// the initial specified amount may also be fixed at issue, but that
/// choice is left to the caller.
///
/// 'TgtPremMonthlyPolFee' is applied here, not in GetModalTgtPrem(),
/// because it is more appropriate here. In the other two cases that
/// GetModalPrem() contemplates:
///  - 'oe_monthly_deduction': deductions would naturally include any
///    policy fee;
///  - 'oe_modal_nonmec': 7702A seven-pay premiums are net by their
///    nature; if it is nonetheless desired to add a policy fee to a
///    (conservative) table-derived 7pp, then 'oe_modal_table' should
///    be used instead.
/// Therefore, an assertion (where 'TgtPremMonthlyPolFee' is assiged)
/// requires that the fee be zero in those cases, and also fires if
/// this function is used for minimum premium with a nonzero fee
/// (because no GetModalPremMinFromTable() has yet been written).
///
/// As the GetModalSpecAmt() documentation for 'oe_modal_table' says,
/// target and minimum premiums really ought to distinguished.

double BasicValues::GetModalPremTgtFromTable
    (int      // a_year // Unused.
    ,mcenum_mode a_mode
    ,double      a_specamt
    ) const
{
    return round_max_premium()
        (
            (   TgtPremMonthlyPolFee * 12
            +       a_specamt
                *   ldbl_eps_plus_one()
                *   MortalityRates_->TargetPremiumRates()[0]
            )
        /   a_mode
        );
}

/// Calculate premium using a corridor ratio.
///
/// Only the initial corridor factor is used here, because this
/// strategy makes sense only at issue. Thus, arguments should
/// represent initial specified amount and mode.

double BasicValues::GetModalPremCorridor
    (int      // a_year // Unused.
    ,mcenum_mode a_mode
    ,double      a_specamt
    ) const
{
    double temp = GetCorridorFactor()[0];
    return round_max_premium()((ldbl_eps_plus_one() * a_specamt / temp) / a_mode);
}

//============================================================================
double BasicValues::GetModalPremGLP
    (int         a_duration
    ,mcenum_mode a_mode
    ,double      a_bft_amt
    ,double      a_specamt
    ) const
{
    // TAXATION !! Use GetAnnualTgtPrem() to get target here if needed
    // for GPT reimplementation.
    double z = Irc7702_->CalculateGLP
        (a_duration
        ,a_bft_amt
        ,a_specamt
        ,Irc7702_->GetLeastBftAmtEver()
        ,effective_dbopt_7702(DeathBfts_->dbopt()[0], Equiv7702DBO3)
        );

// TODO ?? TAXATION !! PROBLEMS HERE
// what if a_year != 0 ?
// term rider, dumpin

    z /= a_mode;
    return round_max_premium()(ldbl_eps_plus_one() * z);
}

//============================================================================
double BasicValues::GetModalPremGSP
    (int         a_duration
    ,mcenum_mode a_mode
    ,double      a_bft_amt
    ,double      a_specamt
    ) const
{
    double z = Irc7702_->CalculateGSP
        (a_duration
        ,a_bft_amt
        ,a_specamt
        ,Irc7702_->GetLeastBftAmtEver()
        );

// TODO ?? TAXATION !! PROBLEMS HERE
// what if a_year != 0 ?
// term rider, dumpin

    z /= a_mode;
    return round_max_premium()(ldbl_eps_plus_one() * z);
}

/// Determine an approximate "pay as you go" modal premium.
///
/// Rationale for overloading this function and its siblings. The
/// simple three-argument form is intended for general use, parallel
/// to similar 'GetModalPrem*' functions; it simply forwards to the
/// four-argument implementation. That four-argument version, with an
/// extra yare_input argument, is intended for calculating premiums
/// under scenarios that differ from actual input, motivated by the
/// group premium quote's need for premiums with and without certain
/// riders. Another means to the same end would be to add an argument
/// for each rider of interest, but that set of riders might expand,
/// whereas the chosen technique is more future-proof.

double BasicValues::GetModalPremMlyDed
    (int         a_year
    ,mcenum_mode a_mode
    ,double      a_specamt
    ) const
{
    return GetModalPremMlyDed(a_year, a_mode, a_specamt, yare_input_);
}

double BasicValues::GetModalPremMlyDedEe
    (int         a_year
    ,mcenum_mode a_mode
    ,double      a_specamt
    ) const
{
    return GetModalPremMlyDedEe(a_year, a_mode, a_specamt, yare_input_);
}

double BasicValues::GetModalPremMlyDedEr
    (int         a_year
    ,mcenum_mode a_mode
    ,double      a_specamt
    ) const
{
    return GetModalPremMlyDedEr(a_year, a_mode, a_specamt, yare_input_);
}

/// Determine an approximate "pay as you go" modal premium.
///
/// This more or less represents actual monthly deductions, at least
/// for monthly mode on an option B contract, generally favoring
/// sufficiency over minimality, but simplicity most of all.
///
/// For simplicity, certain details are disregarded:
///   - premium loads are often stratified--the rate used here is
///     likely to be the highest that might apply, but deductions at
///     age 99 may well exceed target
///   - account-value loads (including, but not limited to, M&E
///     charges) are presumed to be overcome by interest
///   - the specified amount is taken as a scalar, which might not
///     reflect any value it assumes elsewhere (e.g., as a result of a
///     strategy, or of an initial minimum due to the corridor), and
///     might not be the same as the basis for the accident benefit or
///     the specified-amount load, especially if it includes any term
///     rider amount
///   - any term rider included as specified amount is treated as
///     though its charges equal the base policy's COI rates
///
/// If annual_policy_fee is not zero, then level premium on any mode
/// other than annual cannot precisely cover monthly deductions due
/// to the fee's uneven incidence.

double BasicValues::GetModalPremMlyDed
    (int               a_year
    ,mcenum_mode       a_mode
    ,double            a_specamt
    ,yare_input const& a_yi
    ) const
{
    double z = a_specamt * DBDiscountRate[a_year];
    z *= GetBandedCoiRates(mce_gen_curr, a_specamt)[a_year];

    if(a_yi.AccidentalDeathBenefit)
        {
        double r = MortalityRates_->AdbRates()[a_year];
        z += r * std::min(a_specamt, AdbLimit);
        }

    if(a_yi.SpouseRider)
        {
        double r = MortalityRates_->SpouseRiderRates(mce_gen_curr)[a_year];
        z += r * a_yi.SpouseRiderAmount;
        }

    if(a_yi.ChildRider)
        {
        double r = MortalityRates_->ChildRiderRates()[a_year];
        z += r * a_yi.ChildRiderAmount;
        }

    if(true) // Written thus for parallelism and to keep 'r' local.
        {
        double r = Loads_->specified_amount_load(mce_gen_curr)[a_year];
        z += r * std::min(a_specamt, SpecAmtLoadLimit);
        }

    z += Loads_->monthly_policy_fee(mce_gen_curr)[a_year];

    double annual_charge = Loads_->annual_policy_fee(mce_gen_curr)[a_year];

    if(a_yi.WaiverOfPremiumBenefit)
        {
        double const r = MortalityRates_->WpRates()[a_year];
        switch(WaiverChargeMethod)
            {
            case oe_waiver_times_specamt:
                {
                z += r * std::min(a_specamt, WpLimit);
                }
                break;
            case oe_waiver_times_deductions:
                {
                z *= 1.0 + r;
                annual_charge *= 1.0 + r;
                }
                break;
            default:
                {
                alarum()
                    << "Case '"
                    << WaiverChargeMethod
                    << "' not found."
                    << LMI_FLUSH
                    ;
                }
            }
        }

    z /= 1.0 - Loads_->target_premium_load_maximum_premium_tax()[a_year];

    z *= GetAnnuityValueMlyDed(a_year, a_mode);

    z += annual_charge;

    return round_min_premium()(z);
}

// The "-Ee" and "-Er" variants are written with preprocessor
// conditionals for ease of comparison to the unsuffixed original.

double BasicValues::GetModalPremMlyDedEe
    (int               a_year
    ,mcenum_mode       a_mode
    ,double            a_specamt
    ,yare_input const& a_yi
    ) const
{
    double z = a_specamt * DBDiscountRate[a_year];
    z *= GetCurrentTermRates()[a_year];
#if 0
    if(a_yi.AccidentalDeathBenefit)
        {
        double r = MortalityRates_->AdbRates()[a_year];
        z += r * std::min(a_specamt, AdbLimit);
        }
#endif // 0
    if(a_yi.SpouseRider)
        {
        double r = MortalityRates_->SpouseRiderRates(mce_gen_curr)[a_year];
        z += r * a_yi.SpouseRiderAmount;
        }

    if(a_yi.ChildRider)
        {
        double r = MortalityRates_->ChildRiderRates()[a_year];
        z += r * a_yi.ChildRiderAmount;
        }

#if 0
    if(true) // Written thus for parallelism and to keep 'r' local.
        {
        double r = Loads_->specified_amount_load(mce_gen_curr)[a_year];
        z += r * std::min(a_specamt, SpecAmtLoadLimit);
        }

    z += Loads_->monthly_policy_fee(mce_gen_curr)[a_year];

    double annual_charge = Loads_->annual_policy_fee(mce_gen_curr)[a_year];
#endif // 0

    if(a_yi.WaiverOfPremiumBenefit)
        {
        double const r = MortalityRates_->WpRates()[a_year];
        switch(WaiverChargeMethod)
            {
            case oe_waiver_times_specamt:
                {
                z += r * std::min(a_specamt, WpLimit);
                }
                break;
            case oe_waiver_times_deductions:
                {
                z *= 1.0 + r;
#if 0
                annual_charge *= 1.0 + r;
#endif // 0
                }
                break;
            default:
                {
                alarum()
                    << "Case '"
                    << WaiverChargeMethod
                    << "' not found."
                    << LMI_FLUSH
                    ;
                }
            }
        }

    z /= 1.0 - Loads_->target_premium_load_maximum_premium_tax()[a_year];

    double u = DBDiscountRate[a_year];
    z *= (1.0 - std::pow(u, 12.0 / a_mode)) / (1.0 - u);

#if 0
    z += annual_charge;
#endif // 0

    return round_min_premium()(z);
}

double BasicValues::GetModalPremMlyDedEr
    (int               a_year
    ,mcenum_mode       a_mode
    ,double            a_specamt
    ,yare_input const& a_yi
    ) const
{
    double z = a_specamt * DBDiscountRate[a_year];
    z *= GetBandedCoiRates(mce_gen_curr, a_specamt)[a_year];

    if(a_yi.AccidentalDeathBenefit)
        {
        double r = MortalityRates_->AdbRates()[a_year];
        z += r * std::min(a_specamt, AdbLimit);
        }

#if 0
    if(a_yi.SpouseRider)
        {
        double r = MortalityRates_->SpouseRiderRates(mce_gen_curr)[a_year];
        z += r * a_yi.SpouseRiderAmount;
        }

    if(a_yi.ChildRider)
        {
        double r = MortalityRates_->ChildRiderRates()[a_year];
        z += r * a_yi.ChildRiderAmount;
        }
#endif // 0

    if(true) // Written thus for parallelism and to keep 'r' local.
        {
        double r = Loads_->specified_amount_load(mce_gen_curr)[a_year];
        z += r * std::min(a_specamt, SpecAmtLoadLimit);
        }

    z += Loads_->monthly_policy_fee(mce_gen_curr)[a_year];

    double annual_charge = Loads_->annual_policy_fee(mce_gen_curr)[a_year];

    if(a_yi.WaiverOfPremiumBenefit)
        {
        double const r = MortalityRates_->WpRates()[a_year];
        switch(WaiverChargeMethod)
            {
            case oe_waiver_times_specamt:
                {
                z += r * std::min(a_specamt, WpLimit);
                }
                break;
            case oe_waiver_times_deductions:
                {
                z *= 1.0 + r;
                annual_charge *= 1.0 + r;
                }
                break;
            default:
                {
                alarum()
                    << "Case '"
                    << WaiverChargeMethod
                    << "' not found."
                    << LMI_FLUSH
                    ;
                }
            }
        }

    z /= 1.0 - Loads_->target_premium_load_maximum_premium_tax()[a_year];

    double u = DBDiscountRate[a_year];
    z *= (1.0 - std::pow(u, 12.0 / a_mode)) / (1.0 - u);

    z += annual_charge;

    return round_min_premium()(z);
}

//============================================================================
double BasicValues::GetModalSpecAmtMax(double annualized_pmt) const
{
    return GetModalSpecAmt(annualized_pmt, MinPremType);
}

//============================================================================
double BasicValues::GetModalSpecAmtTgt(double annualized_pmt) const
{
    return GetModalSpecAmt(annualized_pmt, TgtPremType);
}

/// Calculate specified amount as a simple function of premium.
///
/// A choice of several such simple functions is offered here to avoid
/// code duplication in GetModalSpecAmtMax() and GetModalSpecAmtTgt().
/// SOMEDAY !! However, in the 'oe_modal_table' case, distinct target
/// and minimum tables and policy fees should be provided instead, and
/// the present implementation moved into the calling functions.
///
/// Argument 'annualized_pmt' is net of any policy fee, such as might
/// be included in a target premium. It's only a scalar, intended to
/// represent an initial premium; reason: it's generally inappropriate
/// for a specified-amount strategy to produce a result that varies by
/// duration.

double BasicValues::GetModalSpecAmt
    (double                annualized_pmt
    ,oenum_modal_prem_type premium_type
    ) const
{
    if(oe_monthly_deduction == premium_type)
        {
        return GetModalSpecAmtMlyDed(annualized_pmt, mce_annual);
        }
    else if(oe_modal_nonmec == premium_type)
        {
        return GetModalSpecAmtMinNonMec(annualized_pmt);
        }
    else if(oe_modal_table == premium_type)
        {
        return round_min_specamt()
            (
                (annualized_pmt - TgtPremMonthlyPolFee * 12)
            /   MortalityRates_->TargetPremiumRates()[0]
            );
        }
    else
        {
        alarum()
            << "Unknown modal premium type " << premium_type << '.'
            << LMI_FLUSH
            ;
        }
    return 0.0;
}

/// Calculate specified amount using a seven-pay ratio.
///
/// Only the initial seven-pay premium rate is used here. Material
/// changes dramatically complicate the relationship between premium
/// and specified amount.

double BasicValues::GetModalSpecAmtMinNonMec(double annualized_pmt) const
{
    // TAXATION !! No table available if 7PP calculated from first principles.
    return round_min_specamt()(annualized_pmt / MortalityRates_->SevenPayRates()[0]);
}

//============================================================================
double BasicValues::GetModalSpecAmtGLP(double annualized_pmt) const
{
    mcenum_dbopt_7702 const z = effective_dbopt_7702(DeathBfts_->dbopt()[0], Equiv7702DBO3);
    return gpt_specamt::CalculateGLPSpecAmt(*this, 0, annualized_pmt, z);
}

//============================================================================
double BasicValues::GetModalSpecAmtGSP(double annualized_pmt) const
{
    return gpt_specamt::CalculateGSPSpecAmt(*this, 0, annualized_pmt);
}

/// Calculate specified amount using a corridor ratio.
///
/// Only the initial corridor factor is used here, because this
/// strategy makes sense only at issue. Thus, arguments should
/// represent initial premium and mode.

double BasicValues::GetModalSpecAmtCorridor(double annualized_pmt) const
{
    return round_min_specamt()(annualized_pmt * GetCorridorFactor()[0]);
}

/// Calculate specified amount based on salary.
///
/// The result of a salary-based strategy is constrained to be
/// nonnegative, because if 'SalarySpecifiedAmountOffset' is
/// sufficiently large, then specamt would be negative, which cannot
/// make any sense.

double BasicValues::GetModalSpecAmtSalary(int a_year) const
{
    double z =
          yare_input_.ProjectedSalary[a_year]
        * yare_input_.SalarySpecifiedAmountFactor
        ;
    if(0.0 != yare_input_.SalarySpecifiedAmountCap)
        {
        z = std::min(z, yare_input_.SalarySpecifiedAmountCap);
        }
    z -= yare_input_.SalarySpecifiedAmountOffset;
    return round_min_specamt()(std::max(0.0, z));
}

/// In general, strategies linking specamt and premium commute. The
/// "pay deductions" strategy, however, doesn't have a useful analog
/// for determining specamt as a function of initial premium: the
/// contract would almost certainly lapse after one year. Therefore,
/// calling this function elicits an error message. SOMEDAY !! It
/// would be better to disable this strategy in the GUI.

double BasicValues::GetModalSpecAmtMlyDed(double, mcenum_mode) const
{
    alarum()
        << "No maximum specified amount is defined for this product."
        << LMI_FLUSH
        ;
    return 0.0;
}

/// 'Unusual' banding is one particular approach we needed to model.
/// Simpler than the banding method generally used in the industry, it
/// determines a single COI rate from the total specified amount and
/// applies that single rate to the entire NAAR. No layers of coverage
/// are distinguished.

std::vector<double> const& BasicValues::GetBandedCoiRates
    (mcenum_gen_basis rate_basis
    ,double           a_specamt
    ) const
{
    if(UseUnusualCOIBanding && mce_gen_guar != rate_basis)
        {
        if(CurrCoiTable0Limit <= a_specamt && a_specamt < CurrCoiTable1Limit)
            {
            return MortalityRates_->MonthlyCoiRatesBand1(rate_basis);
            }
        else if(CurrCoiTable1Limit <= a_specamt)
            {
            return MortalityRates_->MonthlyCoiRatesBand2(rate_basis);
            }
        else
            {
            return MortalityRates_->MonthlyCoiRatesBand0(rate_basis);
            }
        }
    else
        {
        return MortalityRates_->MonthlyCoiRatesBand0(rate_basis);
        }
}

/// Calculate a special modal factor on the fly.
///
/// This factor depends on the general-account rate, which is always
/// specified, even for separate-account-only products.
///
/// This concept is at the same time overelaborate and inadequate.
/// If the crediting rate changes during a policy year, it results in
/// a "pay-deductions" premium that varies between anniversaries, yet
/// may not prevent the contract from lapsing; both those outcomes are
/// likely to frustrate customers.

double BasicValues::GetAnnuityValueMlyDed(int a_year, mcenum_mode a_mode) const
{
    LMI_ASSERT(0.0 != a_mode);
    double spread = 0.0;
    if(mce_monthly != a_mode)
        {
        spread = MinPremIntSpread_[a_year] * 1.0 / a_mode;
        }
    double z = i_upper_12_over_12_from_i<double>()
        (   yare_input_.GeneralAccountRate[a_year]
        -   spread
        );
    double u = 1.0 + std::max
        (z
        ,InterestRates_->GenAcctNetRate
            (mce_gen_guar
            ,mce_monthly_rate
            )[a_year]
        );
    u = 1.0 / u;
    return (1.0 - std::pow(u, 12.0 / a_mode)) / (1.0 - u);
}

/// This forwarding function prevents the 'actuarial_table' module
/// from needing to know about calendar dates and the database.
///
/// At present, exotic lookup methods apply only to current COI rates.
/// An argument could be made for applying them to term rider rates as
/// well.

std::vector<double> BasicValues::GetActuarialTable
    (std::string const& TableFile
    ,e_database_key     TableID
    ,long int           TableNumber
    ) const
{
    if(DB_CurrCoiTable == TableID && e_reenter_never != CoiInforceReentry)
        {
        return actuarial_table_rates_elaborated
            (TableFile
            ,TableNumber
            ,GetIssueAge()
            ,GetLength()
            ,CoiInforceReentry
            ,yare_input_.InforceYear
            ,duration_ceiling(yare_input_.EffectiveDate, yare_input_.LastCoiReentryDate)
            );
        }
    else
        {
        return actuarial_table_rates
            (TableFile
            ,TableNumber
            ,GetIssueAge()
            ,GetLength()
            );
        }
}

//============================================================================
std::vector<double> BasicValues::GetUnblendedTable
    (std::string const& TableFile
    ,e_database_key     TableID
    ) const
{
    return GetActuarialTable
        (TableFile
        ,TableID
        ,static_cast<long int>(Database_->Query(TableID))
        );
}

//============================================================================
std::vector<double> BasicValues::GetUnblendedTable
    (std::string const& TableFile
    ,e_database_key     TableID
    ,mcenum_gender      gender
    ,mcenum_smoking     smoking
    ) const
{
    database_index index = Database_->index().gender(gender).smoking(smoking);
    return GetActuarialTable
        (TableFile
        ,TableID
        ,static_cast<long int>(Database_->Query(TableID, index))
        );
}

//============================================================================
// TODO ?? Better to pass vector as arg rather than return it ?

// This function automatically performs blending by gender and smoking if
// called for. The CanBlend argument tells whether blending is to be
// suppressed for a particular table; its default is to suppress blending.
// For instance, guaranteed COIs might use 80CSO table D for all blends,
// while current COIs reflect the actual blending percentages.
//
// Blending is performed only as called for by the values of the members
// BlendMortSmoking and BlendMortGender of class InputParms and by the
// corresponding arguments.
//
// There are four cases to handle, as can best be seen in a table:
//             female  male  unisex
// smoker         1      1      3
// nonsmoker      1      1      3
// unismoke       2      2      4
//
// The order of blending in the unisex unismoke case makes no difference.
std::vector<double> BasicValues::GetTable
    (std::string const& TableFile
    ,e_database_key     TableID
    ,bool               IsTableValid
    ,EBlend      const& CanBlendSmoking
    ,EBlend      const& CanBlendGender
    ) const
{
    if(!IsTableValid)
        {
        return std::vector<double>(GetLength());
        }

    std::string const file_name = AddDataDir(TableFile);

    // To blend by either smoking or gender, both the input must allow
    // it (yare_input_), and the table must allow it (arg: CanBlend);
    // or it must be required by (arg: MustBlend).
    bool BlendSmoking = false;
    switch(CanBlendSmoking)
        {
        case CannotBlend:
            {
            BlendSmoking = false;
            }
            break;
        case CanBlend:
            {
            BlendSmoking = yare_input_.BlendSmoking;
            }
            break;
        case MustBlend:
            {
            BlendSmoking = true;
            }
            break;
        default:
            {
            alarum()
                << "Case '"
                << CanBlendSmoking
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    bool BlendGender = false;
    switch(CanBlendGender)
        {
        case CannotBlend:
            {
            BlendGender = false;
            }
            break;
        case CanBlend:
            {
            BlendGender = yare_input_.BlendGender;
            }
            break;
        case MustBlend:
            {
            BlendGender = true;
            }
            break;
        default:
            {
            alarum()
                << "Case '"
                << CanBlendGender
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    // Case 1: blending is not allowed or not requested--return unblended table
    if
        (
                !BlendSmoking
            &&  !BlendGender

/*
            !CanBlend
        ||  (
                !yare_input_.BlendSmoking
            &&  !yare_input_.BlendGender
            )
*/
        )
        {
        return GetUnblendedTable(file_name, TableID);
        }

    // Any other case needs this
    std::vector<double> BlendedTable;

    // Case 2: blend by smoking only
    // no else because above if returned
    if
        (
            BlendSmoking
        &&  !BlendGender
        )
        {
        std::vector<double> S = GetUnblendedTable
            (file_name
            ,TableID
            ,yare_input_.Gender
            ,mce_smoker
            );
        std::vector<double> N = GetUnblendedTable
            (file_name
            ,TableID
            ,yare_input_.Gender
            ,mce_nonsmoker
            );
        double n = yare_input_.NonsmokerProportion;
        double s = 1.0 - n;
        for(int j = 0; j < GetLength(); j++)
            {
            BlendedTable.push_back(s * S[j] + n * N[j]);
            }
        }

    // Case 3: blend by gender only
    else if
        (
            !BlendSmoking
        &&  BlendGender
        )
        {
        std::vector<double> F = GetUnblendedTable
            (file_name
            ,TableID
            ,mce_female
            ,yare_input_.Smoking
            );
        std::vector<double> M = GetUnblendedTable
            (file_name
            ,TableID
            ,mce_male
            ,yare_input_.Smoking
            );
        double m = yare_input_.MaleProportion;
        double f = 1.0 - m;

/*
    This approach is better actuarial science, but some products'
    specifications do not do this.
        double f_tpx = 1.0;
        double m_tpx = 1.0;
        double tpx;
        double tpx_prev = 1.0;
        for(int j = 0; j < GetLength(); j++)
            {
            f_tpx *= (1 - F[j]);
            m_tpx *= (1 - M[j]);
            tpx = (f * f_tpx + m * m_tpx);
            BlendedTable.push_back(1.0 - tpx / tpx_prev);
            tpx_prev = tpx;
            }
*/

///*
        for(int j = 0; j < GetLength(); j++)
            {
            BlendedTable.push_back(f * F[j] + m * M[j]);
            }
//*/
        }

    // Case 4: blend by both smoking and gender
    else if
        (
            BlendSmoking
        &&  BlendGender
        )
        {
        std::vector<double> FS = GetUnblendedTable
            (file_name
            ,TableID
            ,mce_female
            ,mce_smoker
            );
        std::vector<double> FN = GetUnblendedTable
            (file_name
            ,TableID
            ,mce_female
            ,mce_nonsmoker
            );
        std::vector<double> MS = GetUnblendedTable
            (file_name
            ,TableID
            ,mce_male
            ,mce_smoker
            );
        std::vector<double> MN = GetUnblendedTable
            (file_name
            ,TableID
            ,mce_male
            ,mce_nonsmoker
            );
        double n = yare_input_.NonsmokerProportion;
        double s = 1.0 - n;
        double m = yare_input_.MaleProportion;
        double f = 1.0 - m;
        for(int j = 0; j < GetLength(); j++)
            {
            BlendedTable.push_back
                (   f * (s * FS[j] + n * FN[j])
                +   m * (s * MS[j] + n * MN[j])
                );
/* Equivalently we could do this:
                (   s * (f * FS[j] + m * MS[j])
                +   n * (f * FN[j] + m * MN[j])
                );
*/
            }
        }
    else
        {
        alarum() << "Invalid mortality blending." << LMI_FLUSH;
        }

    return BlendedTable;
}

//============================================================================
// TAXATION !! Resolve these issues:
// TODO ?? This might be reworked to go through class Irc7702 all the time;
// at least it shouldn't refer to the input class.
// TODO ?? The profusion of similar names should be trimmed.
std::vector<double> const& BasicValues::GetCorridorFactor() const
{
    switch(yare_input_.DefinitionOfLifeInsurance)
        {
        case mce_cvat:
            {
            return MortalityRates_->CvatCorridorFactors();
            }
        case mce_gpt:
            {
            return Irc7702_->Corridor();
            }
        case mce_noncompliant:
            {
            Non7702CompliantCorridor = std::vector<double>(GetLength(), 1.0);
            return Non7702CompliantCorridor;
            }
        default:
            {
            alarum()
                << "Case "
                << yare_input_.DefinitionOfLifeInsurance
                << " not found."
                << LMI_FLUSH
                ;
            }
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

// potential inlines

std::vector<double> const& BasicValues::SpreadFor7702() const
{
    return SpreadFor7702_;
}

std::vector<double> const& BasicValues::GetMly7702iGlp() const
{
    return Mly7702iGlp;
}

std::vector<double> const& BasicValues::GetMly7702qc() const
{
    return Mly7702qc;
}

std::vector<double> const& BasicValues::GetMlyDcvqc() const
{
    return MlyDcvqc;
}

std::vector<double> BasicValues::GetCvatCorridorFactors() const
{
    return GetTable
        (ProductData_->datum("CvatCorridorFilename")
        ,DB_CorridorTable
        );
}

std::vector<double> BasicValues::GetIrc7702NspRates() const
{
    return GetTable
        (ProductData_->datum("Irc7702NspFilename")
        ,DB_CorridorTable
        );
}

// Only current (hence midpoint) COI and term rates are blended.

std::vector<double> BasicValues::GetCurrCOIRates0() const
{
    return GetTable
        (ProductData_->datum("CurrCOIFilename")
        ,DB_CurrCoiTable
        ,true
        ,CanBlend
        ,CanBlend
        );
}

std::vector<double> BasicValues::GetCurrCOIRates1() const
{
    return GetTable
        (ProductData_->datum("CurrCOIFilename")
        ,DB_CurrCoiTable1
        ,CurrCoiTable0Limit < std::numeric_limits<double>::max()
        ,CanBlend
        ,CanBlend
        );
}

std::vector<double> BasicValues::GetCurrCOIRates2() const
{
    return GetTable
        (ProductData_->datum("CurrCOIFilename")
        ,DB_CurrCoiTable2
        ,CurrCoiTable1Limit < std::numeric_limits<double>::max()
        ,CanBlend
        ,CanBlend
        );
}

std::vector<double> BasicValues::GetGuarCOIRates() const
{
    return GetTable
        (ProductData_->datum("GuarCOIFilename")
        ,DB_GuarCoiTable
        );
}

std::vector<double> BasicValues::GetSmokerBlendedGuarCOIRates() const
{
    return GetTable
        (ProductData_->datum("GuarCOIFilename")
        ,DB_GuarCoiTable
        ,true
        ,CanBlend
        ,CanBlend
        );
}

std::vector<double> BasicValues::GetWpRates() const
{
    return GetTable
        (ProductData_->datum("WPFilename")
        ,DB_WpTable
        ,Database_->Query(DB_AllowWp)
        );
}

std::vector<double> BasicValues::GetAdbRates() const
{
    return GetTable
        (ProductData_->datum("ADDFilename")
        ,DB_AdbTable
        ,Database_->Query(DB_AllowAdb)
        );
}

std::vector<double> BasicValues::GetChildRiderRates() const
{
    return GetTable
        (ProductData_->datum("ChildRiderFilename")
        ,DB_ChildRiderTable
        ,Database_->Query(DB_AllowChildRider)
        );
}

std::vector<double> BasicValues::GetCurrentSpouseRiderRates() const
{
    if(!Database_->Query(DB_AllowSpouseRider))
        {
        return std::vector<double>(GetLength());
        }

    std::vector<double> z = actuarial_table_rates
        (AddDataDir(ProductData_->datum("CurrSpouseRiderFilename"))
        ,static_cast<long int>(Database_->Query(DB_SpouseRiderTable))
        ,yare_input_.SpouseIssueAge
        ,EndtAge - yare_input_.SpouseIssueAge
        );
    z.resize(Length);
    return z;
}

std::vector<double> BasicValues::GetGuaranteedSpouseRiderRates() const
{
    if(!Database_->Query(DB_AllowSpouseRider))
        {
        return std::vector<double>(GetLength());
        }

    std::vector<double> z = actuarial_table_rates
        (AddDataDir(ProductData_->datum("GuarSpouseRiderFilename"))
        ,static_cast<long int>(Database_->Query(DB_SpouseRiderGuarTable))
        ,yare_input_.SpouseIssueAge
        ,EndtAge - yare_input_.SpouseIssueAge
        );
    z.resize(Length);
    return z;
}

std::vector<double> BasicValues::GetCurrentTermRates() const
{
    return GetTable
        (ProductData_->datum("CurrTermFilename")
        ,DB_TermTable
        ,Database_->Query(DB_AllowTerm)
        ,CanBlend
        ,CanBlend
        );
}

std::vector<double> BasicValues::GetGuaranteedTermRates() const
{
    return GetTable
        (ProductData_->datum("GuarTermFilename")
        ,DB_GuarTermTable
        ,Database_->Query(DB_AllowTerm)
        ,CanBlend
        ,CanBlend
        );
}

std::vector<double> BasicValues::GetGroupProxyRates() const
{
    return GetTable
        (ProductData_->datum("GroupProxyFilename")
        ,DB_GroupProxyRateTable
        );
}

std::vector<double> BasicValues::GetSevenPayRates() const
{
    return GetTable
        (ProductData_->datum("SevenPayFilename")
        ,DB_SevenPayTable
    // TAXATION !! No table available if 7PP calculated from first principles.
//        ,1 == Database_->Query(DB_SevenPayWhence)
        );
}

std::vector<double> BasicValues::GetTgtPremRates() const
{
    return GetTable
        (ProductData_->datum("TgtPremFilename")
        ,DB_TgtPremTable
// Use this line instead:
//      ,oe_modal_table == TgtPremType
// once the comment concerning GetModalPremMinFromTable()
// in GetModalPremTgtFromTable() is addressed. Meanwhile, this kludge
// permits table-drive minimum premiums in certain circumstances.
        ,oe_modal_table == TgtPremType || oe_modal_table == MinPremType
// To fix this properly, implement a new GetModalPremTgtFromTable();
// separate 'oe_modal_table' into distinct enumerators such as
// 'oe_modal_min_table' and 'oe_modal_tgt_table'; and add a minimum-
// premium rate table. More attention should be paid to the conditions
// under which minimum and target premiums are recalculated--e.g.,
// they might change iff specamt changes.
        );
}

std::vector<double> BasicValues::GetIrc7702QRates() const
{
    return GetTable
        (ProductData_->datum("Irc7702QFilename")
        ,DB_Irc7702QTable
        );
}

std::vector<double> BasicValues::GetPartialMortalityRates() const
{
    return GetTable
        (ProductData_->datum("PartialMortalityFilename")
        ,DB_PartialMortTable
        ,true
        ,CannotBlend
        ,CanBlend
        );
}

std::vector<double> BasicValues::GetSubstdTblMultTable() const
{
    if(0 == Database_->Query(DB_SubstdTableMultTable))
        {
        return std::vector<double>(GetLength(), 1.0);
        }

    return GetTable
        (ProductData_->datum("SubstdTblMultFilename")
        ,DB_SubstdTableMultTable
        );
}

std::vector<double> BasicValues::GetCurrSpecAmtLoadTable() const
{
    return GetTable
        (ProductData_->datum("CurrSpecAmtLoadFilename")
        ,DB_CurrSpecAmtLoadTable
        ,0 != Database_->Query(DB_CurrSpecAmtLoadTable)
        );
}

std::vector<double> BasicValues::GetGuarSpecAmtLoadTable() const
{
    return GetTable
        (ProductData_->datum("GuarSpecAmtLoadFilename")
        ,DB_GuarSpecAmtLoadTable
        ,0 != Database_->Query(DB_GuarSpecAmtLoadTable)
        );
}

