<?xml version="1.0" encoding="UTF-8"?>
<!--
    Life insurance illustrations.

    Copyright (C) 2004, 2005, 2006, 2007 Gregory W. Chicares.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA

    http://savannah.nongnu.org/projects/lmi
    email: <chicares@cox.net>
    snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

    $Id: illustration_reg.xsl,v 1.52 2007-07-03 11:24:06 etarassov Exp $
-->
<!DOCTYPE stylesheet [
<!ENTITY nbsp "&#xA0;">
]>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" version="1.0">
  <xsl:import href="fo_common.xsl"/>
  <xsl:output method="xml" encoding="UTF-8" indent="yes"/>
  <xsl:variable name="counter" select="1"/>
  <xsl:variable name="inforceyear" select="$scalars/InforceYear"/>
  <xsl:variable name="lcletters">abcdefghijklmnopqrstuvwxyz</xsl:variable>
  <xsl:variable name="ucletters">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>
  <xsl:variable name="ModifiedSinglePremium">
    <xsl:call-template name="set_modified_single_premium"/>
  </xsl:variable>
  <xsl:variable name="SinglePremium">
    <xsl:call-template name="set_single_premium"/>
  </xsl:variable>
  <xsl:variable name="GroupExperienceRating">
    <xsl:call-template name="set_group_experience_rating"/>
  </xsl:variable>

  <xsl:template match="/">
    <fo:root>
      <fo:layout-master-set>

        <!-- Define the cover page. -->
        <fo:simple-page-master master-name="cover" margin=".1in .35in 0">
          <xsl:call-template name="set-page-size"/>
          <fo:region-body margin=".25in 0 .1in"/>
        </fo:simple-page-master>

        <!-- Define the narrative summary page. -->
        <fo:simple-page-master master-name="narrative-summary" margin=".25in">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin=".5in 0 1in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent=".5in"/>
        </fo:simple-page-master>

        <!-- Define the column headings and key terms page. -->
        <fo:simple-page-master master-name="column-headings-and-key-terms" margin=".25in">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin=".15in 0 .45in"/>
          <!-- Header -->
          <fo:region-before extent="2in"/>
          <!-- Footer -->
          <fo:region-after extent="0.45in"/>
        </fo:simple-page-master>

        <!-- Define the Numeric Summary page. -->
        <fo:simple-page-master master-name="numeric-summary" margin=".25in">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="3.1in 0 .52in"/>
          <!-- Header -->
          <fo:region-before extent="3.1in"/>
          <!-- Footer -->
          <fo:region-after extent="0.52in"/>
        </fo:simple-page-master>

        <!-- Define the Tabular Detail page. -->
        <fo:simple-page-master master-name="tabular-detail" margin=".25in">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="3.1in 0 1.3in"/>
          <!-- Header -->
          <fo:region-before extent="3.1in"/>
          <!-- Footer -->
          <fo:region-after extent="1.27in"/>
        </fo:simple-page-master>

        <!-- Define the Tabular Detail (Report 2) page. -->
        <fo:simple-page-master master-name="tabular-detail-report2" margin=".25in">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="3.2in 0 1.35in"/>
          <!-- Header -->
          <fo:region-before extent="3.1in"/>
          <!-- Footer -->
          <fo:region-after extent="1.27in"/>
        </fo:simple-page-master>

        <!-- Define the Supplemental Report page. -->
        <xsl:if test="$has_supplemental_report">
          <fo:simple-page-master master-name="supplemental-report" margin=".25in">
            <xsl:call-template name="set-page-size"/>
            <!-- Central part of page -->
            <fo:region-body margin="3in 0 1.25in"/>
            <!-- Header -->
            <fo:region-before extent="3.0in"/>
            <!-- Footer -->
            <fo:region-after extent="1.27in"/>
          </fo:simple-page-master>
        </xsl:if>

        <!-- Define the Numeric Summary Attachment page. -->
        <fo:simple-page-master master-name="numeric-summary-attachment" margin=".25in">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="3.1in 0 .52in"/>
          <!-- Header -->
          <fo:region-before extent="3.1in"/>
          <!-- Footer -->
          <fo:region-after extent="0.52in"/>
        </fo:simple-page-master>

      </fo:layout-master-set>

      <!-- The data to be diplayed in the pages, cover page first -->
      <xsl:call-template name="generic-cover"/>

      <!-- NARRATIVE SUMMARY - begins here -->
      <fo:page-sequence master-reference="narrative-summary" initial-page-number="1">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <xsl:call-template name="company-logo"/>
          </fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter"/>

        <!-- Narrative Summary Body  -->
        <fo:flow flow-name="xsl-region-body">
          <xsl:call-template name="standardheader"/>
          <fo:block text-align="center" font-size="10pt" padding-top="1em">
            NARRATIVE SUMMARY
          </fo:block>
          <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
            <fo:block padding-top="1em">
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$SinglePremium!='1'">
                  <xsl:value-of select="$scalars/PolicyMktgName"/> is a
                  <!-- Group Experience Rating Logic -->
                  <xsl:if test="$GroupExperienceRating='1'">
                    group
                  </xsl:if>
                  flexible premium adjustable life insurance contract.
                  <!-- Group Experience Rating Logic -->
                  <xsl:if test="$GroupExperienceRating='1'">
                    It is a no-load policy and is intended for large case sales.
                    It is primarily marketed to financial institutions
                    to fund certain corporate liabilities.
                  </xsl:if>
                  It features accumulating account values, adjustable benefits,
                  and flexible premiums.
                </xsl:when>
                <xsl:when test="$ModifiedSinglePremium='1'">
                  <xsl:value-of select="$scalars/PolicyMktgName"/>
                  is a modified single premium adjustable life
                  insurance contract. It features accumulating
                  account values, adjustable benefits, and single premium.
                </xsl:when>
                <xsl:otherwise>
                  <xsl:value-of select="$scalars/PolicyMktgName"/>
                  is a single premium adjustable life insurance contract.
                  It features accumulating account values,
                  adjustable benefits, and single premium.
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <fo:block padding-top="1em">
              Coverage may be available on a Guaranteed Standard Issue basis.
              All proposals are based on case characteristics and must
              be approved by the <xsl:value-of select="$scalars/InsCoShortName"/>
              Home Office. For details regarding underwriting
              and coverage limitations refer to your offer letter
              or contact your <xsl:value-of select="$scalars/InsCoShortName"/>
              representative.
            </fo:block>
            <fo:block padding-top="1em">
              This is an illustration only. An illustration is not intended
              to predict actual performance. Interest rates
              <xsl:if test="$scalars/Participating='1'">, dividends,</xsl:if>
              and values set forth in the illustration are not guaranteed.
            </fo:block>
            <!-- Group Experience Rating Logic -->
            <fo:block padding-top="1em">
              <xsl:choose>
                <xsl:when test="$scalars/StatePostalAbbrev!='TX'">
                  This illustration assumes that the currently illustrated
                  non-guaranteed elements will continue unchanged
                  for all years shown. This is not likely to occur
                  and actual results may be more or less favorable than shown.
                  The non-guaranteed benefits and values are not guaranteed
                  and are based on assumptions such as interest credited
                  and current monthly charges, which are subject to change by
                  <xsl:value-of select="$scalars/InsCoName"/>.
                </xsl:when>
                <xsl:otherwise>
                  This illustration is based on both non-guaranteed
                  and guaranteed assumptions. Non-guaranteed assumptions
                  include interest rates and monthly charges.
                  This illustration assumes that the currently illustrated
                  non-guaranteed elements will continue unchanged
                  for all years shown. This is not likely to occur
                  and actual results may be more or less favorable than shown.
                  Factors that may affect future policy performance include
                  the company's expectations for future mortality, investments,
                  persistency, profits and expenses.
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:value-of select="$scalars/AvName"/> Values may be used
              to pay monthly charges. Monthly charges are due during
              the life of the insured, and depending on actual results,
              the premium payor may need to continue or resume premium outlays.
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:choose>
                <!-- Single Premium Logic -->
                <xsl:when test="$SinglePremium!='1'">
                  Premiums are assumed to be paid on
                  a<xsl:if test="$vectors[@name='ErMode']/duration[1]/@column_value='Annual'">n </xsl:if>
                  <xsl:value-of select="translate($vectors[@name='ErMode']/duration[1]/@column_value,$ucletters,$lcletters)"/>
                  basis and received at the beginning of the contract year.
                </xsl:when>
                <xsl:otherwise>
                  The single premium is assumed to be paid at the beginning
                  of the contract year.
                </xsl:otherwise>
              </xsl:choose>
              <xsl:value-of select="$scalars/AvName"/> Values,
              <xsl:value-of select="$scalars/CsvName"/> Values,
              and death benefits are illustrated as of the end
              of the contract year. The method we use to allocate
              overhead expenses is the fully allocated expense method.
            </fo:block>
            <!-- Single Premium Logic -->
            <xsl:if test="$SinglePremium!='1'">
              <fo:block padding-top="1em">
                In order to guarantee coverage to age
                <xsl:value-of select="$scalars/EndtAge"/>,
                a<xsl:if test="$vectors[@name='ErMode']/duration[1]/@column_value='Annual'">n </xsl:if>
                <xsl:value-of select="translate($vectors[@name='ErMode']/duration[1]/@column_value,$ucletters,$lcletters)"/>
                premium
                <xsl:choose>
                  <xsl:when test="$scalars/GuarPrem!='0'">
                    of $<xsl:value-of select="$scalars/GuarPrem"/>
                    must be paid.
                  </xsl:when>
                  <xsl:otherwise>
                    is defined.
                  </xsl:otherwise>
                </xsl:choose>
                This amount is based on the guaranteed monthly charges
                and the guaranteed interest crediting rate.
                <xsl:if test="$scalars/DefnLifeIns='GPT'">
                  This premium may be in excess of the maximum premium allowable
                  in order to qualify this policy as life insurance.
                </xsl:if>
              </fo:block>
            </xsl:if>
            <fo:block padding-top="1em">
              Loaned amounts of the <xsl:value-of select="$scalars/AvName"/>
              Value will be credited a rate equal to the loan interest rate less
              a spread, guaranteed not to exceed 3.00%.
            </fo:block>
            <xsl:if test="$scalars/HasTerm='1'">
              <fo:block padding-top="1em">
                The term rider provides the option to purchase monthly
                term insurance on the life of the insured. The term rider
                selected face amount supplements the selected face amount
                of the contract. If the term rider is attached, the policy
                to which it is attached may have a lower annual cutoff premium
                and, as a result, the lower overall sales loads paid may be
                lower than a contract having the same total face amount,
                but with no term rider.
                <xsl:if test="$scalars/NoLapse='1'">
                  Also, the lapse protection feature of the contract's
                  <xsl:value-of select="$scalars/NoLapseProvisionName"/>
                  does not apply to the term rider's selected face amount.
                </xsl:if>
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/HasWP='1'">
              <fo:block padding-top="1em">
                The Waiver of Monthly Charges Rider provides for waiver
                of monthly charges in the event of the disability
                of the insured that begins before attained age 65
                and continues for at least 6 months, as described in the rider.
                An additional charge is associated with this rider. Please refer
                to your contract for specific provisions and a detailed schedule
                of charges.
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/HasADD='1'">
              <fo:block padding-top="1em">
                The Accidental Death benefit provides an additional benefit
                if death is due to accident. An additional charge is associated
                with this rider. Please refer to your contract
                for specific provisions and a detailed schedule of charges.
              </fo:block>
            </xsl:if>
            <fo:block padding-top="1em">
              The definition of life insurance for this contract is the
              <xsl:choose>
                <xsl:when test="$scalars/DefnLifeIns='GPT'">
                  guideline premium test. The guideline single premium
                  is $<xsl:value-of select="$scalars/InitGSP"/>
                  and the guideline level premium
                  is $<xsl:value-of select="$scalars/InitGLP"/>
                </xsl:when>
                <xsl:otherwise>
                  cash value accumulation test.
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
          </fo:block>

          <!-- Force Second Page -->
          <fo:block break-after="page"/>
          <fo:block text-align="center" font-size="10pt">
            NARRATIVE SUMMARY (Continued)
          </fo:block>
          <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
            <xsl:if test="$scalars/SalesLoadRefund!='0%'">
              <fo:block padding-top="2em">
                Sales Load Refund: We will refund a portion of the sales load
                to you, as part of your <xsl:value-of select="$scalars/CsvName"/>
                Value, if you surrender your contract within the first two
                policy years. In policy year 1, we will refund
                <xsl:value-of select="$scalars/SalesLoadRefundRate0"/>
                of the first contract year sales load collected
                and in contract year 2, we will refund
                <xsl:value-of select="$scalars/SalesLoadRefundRate1"/>
                of the first contract year sales load collected.
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/NoLapse='1'">
              <fo:block padding-top="1em">
                <xsl:value-of select="$scalars/NoLapseProvisionName"/>:
                The contract will remain in force after the first premium
                has been paid, even if there is insufficient
                <xsl:value-of select="$scalars/AvName"/> Value
                to cover the monthly charges provided that the insured
                is not in a substandard rating class and the policy debt
                does not exceed <xsl:value-of select="$scalars/AvName"/> Value.
              </fo:block>
            </xsl:if>

            <fo:block padding-top="1em">
              This contract has a guaranteed maximum cost of insurance
              (based on 1980 CSO mortality tables) and maximum
              administrative charges. The actual current charges are lower
              than these and are reflected in the values.
              However, these current charges are subject to change.
            </fo:block>
            <fo:block padding-top="1em">
              This illustration assumes death of the insured
              at age <xsl:value-of select="$scalars/EndtAge"/>.
            </fo:block>
            <fo:block padding-top="1em">
              The loan interest rate is fixed
              at <xsl:value-of select="$scalars/InitAnnLoanDueRate"/> per year.
            </fo:block>
            <fo:block padding-top="1em">
              The state of issue
              is <xsl:value-of select="$scalars/StatePostalAbbrev"/>.
            </fo:block>
            <xsl:if test="$is_composite">
              <fo:block padding-top="1em">
                Please see the attached census, listing the face amounts,
                underwriting classes and issue ages for individual participants.
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/StatePostalAbbrev='NC' or $scalars/StatePostalAbbrev='SC'">
              <fo:block padding-top="1em">
                In the states of North Carolina and South Carolina,
                Guaranteed Issue Underwriting is referred
                to as "Limited Underwriting" and Simplified
                Issue Underwriting is referred to as "Simplified Underwriting".
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/StatePostalAbbrev='TX'">
              <xsl:if test="$scalars/UWType='Guaranteed issue'">
                <fo:block padding-top="1em">
                  * This policy is classified as substandard guaranteed issue
                  per the requirements of the Texas Insurance Department.
                </fo:block>
              </xsl:if>
            </xsl:if>
            <!-- Group Experience Rating Logic -->
            <xsl:if test="$GroupExperienceRating='1'">
              <fo:block padding-top="1em">
                We may assess a Market Value Adjustment upon a surrender
                of the certificate when the surrender proceeds are intended
                to be applied to an insurance policy issued
                by an insurer unaffilliated with MML Bay State with an intent
                to qualify the exchange as a tax free exchange under IRC
                section 1035.
              </fo:block>
              <xsl:if test="$scalars/UseExperienceRating!='1'">
                <fo:block padding-top="1em">
                  This illustration does not reflect experience rating.
                </fo:block>
              </xsl:if>
              <fo:block padding-top="1em">
                The guaranteed values reflect the maximum charges permitted
                by the contract, which may include an Experience Rating
                Risk Charge.
              </fo:block>
              <fo:block padding-top="1em">
                No Experience Rating Risk Charge or a distribution
                of an Experience Rating Reserve Credit is reflected
                in the current, non-guaranteed values. Actual charges
                and credits will be based on the actual experience of the group.
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/Has1035ExchCharge='1'">
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$SinglePremium!='1'">
                  <xsl:if test="$scalars/Has1035ExchCharge='1'">
                    <fo:block padding-top="1em">
                      Upon surrender of this policy, where the surrender
                      proceeds are intended to be applied to an insurance policy
                      or certificate issued in conjunction with an intent
                      to qualify the exchange as a tax free exchange
                      under Section 1035 of the Internal Revenue Code,
                      we may assess an Exchange Charge. The Exchange Charge
                      is the greater of the Market Value Adjustment Charge
                      and the Percentage of Premium Charge. In the states
                      of Florida or Indiana, the Exchange charge
                      (referred to as Assignment Charge in Florida)
                      will be the Percentage of Premium Charge only.
                      The Exchange Charge will potentially reduce
                      the surrender proceeds, but will never increase
                      the surrender proceeds. Please refer to your contract
                      for details.
                    </fo:block>
                  </xsl:if>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block padding-top="1em">
                    Upon surrender of this policy, where the surrender proceeds
                    are intended to be applied to an insurance policy
                    or certificate issued in conjunction with an intent
                    to qualify the exchange as a tax free exchange
                    under Section 1035 of the Internal Revenue Code,
                    we may assess an Exchange Charge. The Exchange Charge
                    will potentially reduce the surrender proceeds,
                    but will never increase the surrender proceeds.
                    Please refer to your contract for details.
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </xsl:if>
            <xsl:if test="$scalars/HasSpouseRider='1'">
              <fo:block padding-top="1em">
                The $<xsl:value-of select="$scalars/SpouseRiderAmount"/> Spouse
                rider provides term life insurance on the spouse
                (issue age <xsl:value-of select="$scalars/SpouseIssueAge"/>)
                for a limited duration, for an extra charge.
                Please refer to your contract for specific provisions
                and a detailed schedule of charges.
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/HasChildRider='1'">
              <fo:block padding-top="1em">
                The $<xsl:value-of select="$scalars/ChildRiderAmount"/> Child
                rider provides term life insurance on the insured's children
                for a limited duration, for an extra charge. Please refer
                to your contract for specific provisions
                and a detailed schedule of charges.
              </fo:block>
            </xsl:if>
            <fo:block font-weight="bold" text-align="center" padding-top="1em">
              IMPORTANT TAX DISCLOSURE
            </fo:block>
            <fo:block padding-top="1em">
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$SinglePremium!='1'">
                  As illustrated, this contract
                  <xsl:choose>
                    <xsl:when test="$scalars/IsMec='1'">
                      becomes
                    </xsl:when>
                    <xsl:otherwise>
                      would not become
                    </xsl:otherwise>
                  </xsl:choose>
                  a Modified Endowment Contract (MEC)
                  under the Internal Revenue Code
                  <xsl:if test="$scalars/IsMec='1'">
                      in year <xsl:value-of select="$scalars/MecYear+1"/>
                  </xsl:if>.
                  To the extent of gain in the contract, loans, distributions
                  and withdrawals from a MEC are subject to income tax
                  and may also trigger a penalty tax.
                </xsl:when>
                <xsl:otherwise>
                  This contract is a Modified Endowment Contract (MEC)
                  under the Internal Revenue Code. To the extent of gain
                  in the contract, loans, distributions and withdrawals
                  from a MEC are subject to income tax and may also trigger
                  a penalty tax.
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <!-- Single Premium Logic -->
            <xsl:if test="$SinglePremium!='1' and $scalars/IsInforce!='1'">
              <fo:block padding-top="1em">
                The initial 7-pay premium limit
                is $<xsl:value-of select="$scalars/InitSevenPayPrem"/>.
              </fo:block>
            </xsl:if>
            <fo:block font-weight="bold" padding-top="1em">
              The information contained in this illustration is not written
              or intended as tax or legal advice, and may not be relied upon
              for purposes of avoiding any federal tax penalties.
              Neither <xsl:value-of select="$scalars/InsCoShortName"/> nor any
              of its employees or representatives are authorized to give tax
              or legal advice. For more information pertaining
              to the tax consequences of purchasing or owning this policy,
              consult with your own independent tax or legal counsel.
            </fo:block>
            <xsl:choose>
              <xsl:when test="$scalars/IsInforce!='1'">
                <xsl:if test="string-length($scalars/InsCoPhone) &gt; 14">
                  <fo:block padding-top="2em">
                    Compliance tracking number:
                    <xsl:value-of select="substring($scalars/InsCoPhone, 1, 15)"/>
                  </fo:block>
                </xsl:if>
              </xsl:when>
              <xsl:otherwise>
                <xsl:if test="string-length($scalars/InsCoPhone) &gt; 16">
                  <fo:block padding-top="2em">
                    Compliance Tracking Number:
                    <xsl:value-of select="substring($scalars/InsCoPhone, 16)"/>
                  </fo:block>
                </xsl:if>
              </xsl:otherwise>
            </xsl:choose>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>

      <!-- Column Headings and Key Terms - begins here -->
      <fo:page-sequence master-reference="column-headings-and-key-terms">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <xsl:call-template name="company-logo"/>
          </fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter"/>

        <!-- Column Headings and Key Terms Body  -->
        <fo:flow flow-name="xsl-region-body">
          <fo:block text-align="center" font-size="10.0pt" padding-top="1em">
            Column Headings and Key Terms Used in This Illustration
          </fo:block>
          <fo:block text-align="left" font-size="9pt" font-family="sans-serif" padding-top="1em">
            <fo:block>
              <fo:inline font-weight="bold">
                <xsl:value-of select="$scalars/AvName"/> Value:
              </fo:inline>
              The accumulation at interest of the net premiums paid,
              <xsl:if test="$SinglePremium!='1'">
                less any withdrawals,
              </xsl:if>
              less any monthly charges deducted.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                <xsl:value-of select="$scalars/CsvName"/> Value:
              </fo:inline>
              <xsl:value-of select="$scalars/AvName"/> Value less policy debt.
              The <xsl:value-of select="$scalars/CsvName"/> Value
              does not reflect an Exchange Charge, which may be assessed
              under the policy where surrender proceeds are intended
              to be applied to an insurance policy or certificate issued
              with an intent to qualify the exchange as a tax free exchange
              under Section 1035 of the Internal Revenue Code.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Current Values:
              </fo:inline>
              Values assuming current interest crediting rates
              and current monthly charges. These values are not guaranteed
              and are based on the assumption that premium is paid
              as illustrated.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Death Benefit:
              </fo:inline>
              The amount of benefit provided by the Death Benefit Option
              in effect on the date of death, prior to adjustments
              for policy debt and monthly charges payable to the date of death.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Death Benefit Option 1:
              </fo:inline>
              Option in which the death benefit is equal to the selected
              face amount of the contract on the date of death of the insured,
              or if greater the <xsl:value-of select="$scalars/AvName"/> Value
              <xsl:if test="$scalars/SalesLoadRefund!='0%'">
                plus the refund of sales loads (if applicable)
              </xsl:if>
              on the insured's date of death multiplied by the minimum face
              amount percentage for the insured's attained age at death
              (minimum face amount). Please refer to the contract
              for a detailed schedule of death benefit factors.
            </fo:block>
            <!-- Group Experience Rating Logic -->
            <xsl:if test="$GroupExperienceRating!='1'">
              <fo:block padding-top="1em">
                <fo:inline font-weight="bold">
                  Death Benefit Option 2:
                </fo:inline>
                Option in which the death benefit is equal to the selected
                face amount of the contract
                plus the <xsl:value-of select="$scalars/AvName"/> Value
                on the date of death of the insured, or if greater,
                the <xsl:value-of select="$scalars/AvName"/> Value
                <xsl:if test="$scalars/SalesLoadRefund!='0%'">
                  plus the refund of sales loads (if applicable)
                </xsl:if>
                on the insured's date of death multiplied
                by the death benefit factor for the insured's attained age
                at death (minimum face amount). Please refer to the contract
                for a detailed schedule of death benefit factors.
              </fo:block>
            </xsl:if>
            <!-- Group Experience Rating Logic -->
            <xsl:if test="$GroupExperienceRating='1'">
              <fo:block padding-top="1em">
                <fo:inline font-weight="bold">
                  Experience Rating Risk Charge:
                </fo:inline>
                Applies only to certain experience rated groups.
                This charge is based on the cost of insurance charges
                assessed during the certificate year. It may be assessed against
                the account value once per certificate anniversary date
                and upon surrender of the group policy.
              </fo:block>
            </xsl:if>
            <!-- Group Experience Rating Logic -->
            <xsl:if test="$GroupExperienceRating!='1'">
              <fo:block padding-top="1em">
                <fo:inline font-weight="bold">
                  Exchange Charge:
                </fo:inline>
                Where surrender proceeds are intended to be applied
                to an insurance policy or certificate issued with an intent
                to qualify the exchange as a tax free exchange
                under Section 1035 of the Internal Revenue Code,
                there is a potential reduction in surrender proceeds.
                Please see the contract endorsement for a detailed description
                of the Exchange Charge.
              </fo:block>
            </xsl:if>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Flexible Premiums:
              </fo:inline>
              Premiums that may be increased, reduced, or not paid,
              if the account value is sufficient to cover the monthly charges.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Guaranteed Values:
              </fo:inline>
              Values assuming the guaranteed crediting rate
              and the guaranteed maximum monthly charges. These values
              are based on the assumption that premium is paid as illustrated.
            </fo:block>
            <xsl:if test="$scalars/IsInforce!='1'">
              <fo:block padding-top="1em">
                <fo:inline font-weight="bold">
                  Initial Illustrated Crediting Rate:
                </fo:inline>
                The current interest rate illustrated for the first policy year.
                This rate is not guaranteed and is subject
                to change by <xsl:value-of select="$scalars/InsCoName"/>.
              </fo:block>
            </xsl:if>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                MEC:
              </fo:inline>
              Modified Endowment Contract - this classification is given
              to a contract in violation of TAMRA
              (Technical and Miscellaneous Revenues Act), which limits
              the amount of premium that can be paid into a life
              insurance contract. To the extent of gain in the contract, loans,
              distributions and withdrawals from a MEC are subject
              to income tax and may also trigger a tax penalty.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Midpoint Values:
              </fo:inline>
              Values assuming interest rates that are the average
              of the illustrated current crediting rates
              and the guaranteed minimum interest rate, and monthly charges
              that are the average of the current monthly charges
              and the guaranteed monthly charges.
              These values are not guaranteed and are based on the assumption
              that premium is paid as illustrated.
            </fo:block>
            <!-- Single Premium Logic -->
            <xsl:if test="$ModifiedSinglePremium='1'">
              <fo:block padding-top="1em">
                <fo:inline font-weight="bold">
                  Modified Single Premium:
                </fo:inline>
                After the single premium is paid, additional payment
                under this policy will only be accepted for repayment
                of policy debt, payment required to keep the policy
                from lapsing, or payment required to reinstate the policy.
              </fo:block>
            </xsl:if>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Monthly Charges:
              </fo:inline>
              The monthly charges for the following month which include:
              cost of insurance, plus face amount charges (if applicable),
              plus the administrative charge shown
              on the contract schedule page.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Premium Outlay:
              </fo:inline>
              The amount of premium assumed to be paid by the contract owner
              or other premium payor.
            </fo:block>
            <!-- Single Premium Logic -->
            <xsl:if test="$SinglePremium='1' and $ModifiedSinglePremium!='1'">
              <fo:block padding-top="1em">
                <fo:inline font-weight="bold">
                  Single Premium:
                </fo:inline>
                After the single premium is paid, additional payment
                under this policy will only be accepted for repayment
                of policy debt, payment required to keep the policy
                from lapsing, or payment required to reinstate the policy.
              </fo:block>
            </xsl:if>
            <!-- Single Premium Logic -->
            <xsl:if test="$SinglePremium='1'">
              <fo:block padding-top="1em">
                <fo:inline font-weight="bold">
                  Ultimate Illustrated Crediting Rate:
                </fo:inline>
                The current interest rate illustrated for policy years
                6 and later. The illustrated crediting rates for policy years
                2 through 5 are based on a blend of the Initial
                and Ultimate Illustrated Crediting Rates.
                These rates are not guaranteed and are subject
                to change by <xsl:value-of select="$scalars/InsCoName"/>.
              </fo:block>
            </xsl:if>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>

      <xsl:if test="$scalars/IsInforce!='1'">
        <!-- Numeric Summary (only for new business)-->
        <!-- Body page -->
        <fo:page-sequence master-reference="numeric-summary">

          <!-- Define the contents of the header. -->
          <fo:static-content flow-name="xsl-region-before">
            <fo:block text-align="left">
              <xsl:call-template name="company-logo"/>
            </fo:block>
            <xsl:call-template name="standardheader"/>
            <fo:block text-align="center" font-size="10.0pt" space-before="5.0pt">
              <xsl:text>Numeric Summary</xsl:text>
            </fo:block>
            <xsl:call-template name="dollar-units"/>
          </fo:static-content>

          <!-- Define the contents of the footer. -->
          <xsl:call-template name="standardfooter"/>

          <xsl:call-template name="numeric-summary-report"/>
        </fo:page-sequence>
      </xsl:if>

      <!-- Tabular Detail -->
      <!-- Body page -->
      <fo:page-sequence master-reference="tabular-detail">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <xsl:call-template name="company-logo"/>
          </fo:block>
          <xsl:call-template name="standardheader"/>
          <fo:block text-align="center" font-size="10.0pt" space-before="5.0pt">
            <xsl:text>Tabular Detail</xsl:text>
          </fo:block>
          <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter">
          <xsl:with-param name="disclaimer">
            The Non-Guaranteed Values depicted above reflect interest rates described in the Tabular Detail, and current monthly charges. These values
            are not guaranteed and depend upon company experience. Column headings indicate whether benefits and values are guaranteed or not guaranteed. This
            illustration assumes that non-guaranteed elements will continue unchanged for all years shown. This is not likely to occur and actual results may be
            more or less favorable than shown. Non-guaranteed elements are subject to change by the insurer. Factors that may affect future policy performance
            include the company's expectations for future mortality, investments, persistency, profits and expenses.
          </xsl:with-param>
        </xsl:call-template>

        <fo:flow flow-name="xsl-region-body">
          <xsl:variable name="tabular-detail-report-columns">
            <column name="PolicyYear">                               | Policy _Year    </column>
            <column composite="0" name="AttainedAge">                | End of _Year Age</column>
            <column name="GrossPmt">                                 | Premium _Outlay </column>
            <column name="AcctVal_Guaranteed">      Guaranteed Values| Account _Value  </column>
            <column name="CSVNet_Guaranteed">       Guaranteed Values| Cash Surr _Value</column>
            <column name="EOYDeathBft_Guaranteed">  Guaranteed Values| Death _Benefit  </column>
            <column/>
            <column name="AcctVal_Current">     Non-Guaranteed Values| Account _Value  </column>
            <column name="CSVNet_Current">      Non-Guaranteed Values| Cash Surr _Value</column>
            <column name="EOYDeathBft_Current"> Non-Guaranteed Values| Death _Benefit  </column>
          </xsl:variable>
          <xsl:variable name="tabular-detail-report-columns-raw" select="document('')//xsl:variable[@name='tabular-detail-report-columns']/column"/>
          <xsl:variable name="columns" select="$tabular-detail-report-columns-raw[not(@composite) or boolean(boolean(@composite='1')=$is_composite)]"/>

          <fo:block font-size="9.0pt" font-family="serif">
            <fo:table table-layout="fixed" width="100%">
              <xsl:call-template name="generate-table-columns">
                <xsl:with-param name="columns" select="$columns"/>
              </xsl:call-template>

              <fo:table-header>
                <xsl:call-template name="generate-table-headers">
                  <xsl:with-param name="columns" select="$columns"/>
                </xsl:call-template>
              </fo:table-header>

              <fo:table-body>
                <xsl:call-template name="generate-table-values">
                  <xsl:with-param name="columns" select="$columns"/>
                  <xsl:with-param name="counter" select="$scalars/InforceYear + 1"/>
                  <xsl:with-param name="max-counter" select="$max-lapse-year"/>
                  <xsl:with-param name="inforceyear" select="0 - $scalars/InforceYear"/>
                </xsl:call-template>
              </fo:table-body>
            </fo:table>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>
      <!-- Tabular Detail (Report 2)-->
      <!-- Body page -->
      <fo:page-sequence master-reference="tabular-detail-report2">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <xsl:call-template name="company-logo"/>
          </fo:block>
          <xsl:call-template name="standardheader"/>
          <fo:block text-align="center" font-size="10.0pt" space-before="5.0pt">
            <xsl:text>Tabular Detail, continued</xsl:text>
          </fo:block>
          <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter">
          <xsl:with-param name="disclaimer">
            The Non-Guaranteed Values depicted above reflect interest rates described in the Tabular Detail, and current monthly charges. These values
            are not guaranteed and depend upon company experience. Column headings indicate whether benefits and values are guaranteed or not guaranteed. This
            illustration assumes that non-guaranteed elements will continue unchanged for all years shown. This is not likely to occur and actual results may be
            more or less favorable than shown. Non-guaranteed elements are subject to change by the insurer. Factors that may affect future policy performance
            include the company's expectations for future mortality, investments, persistency, profits and expenses.
          </xsl:with-param>
        </xsl:call-template>

        <fo:flow flow-name="xsl-region-body">
          <xsl:variable name="tabular-detail-report2-columns">
            <column name="PolicyYear">Policy _Year</column>
            <column composite="0" name="AttainedAge">End of _Year Age</column>
            <column name="AnnGAIntRate_Current">Illustrated _Crediting Rate</column>
            <column composite="0" name="MonthlyFlatExtra">Annual Flat Extra _per $1,000</column>
          </xsl:variable>
          <xsl:variable name="tabular-detail-report2-columns-raw" select="document('')//xsl:variable[@name='tabular-detail-report2-columns']/column"/>
          <xsl:variable name="columns" select="$tabular-detail-report2-columns-raw[not(@composite) or boolean(boolean(@composite='1')=$is_composite)]"/>

          <fo:block font-size="9.0pt" font-family="serif">
            <fo:table table-layout="fixed" width="{8*count($columns)}em">
              <xsl:call-template name="generate-table-columns">
                <xsl:with-param name="columns" select="$columns"/>
              </xsl:call-template>

              <fo:table-header>
                <xsl:call-template name="generate-table-headers">
                  <xsl:with-param name="columns" select="$columns"/>
                </xsl:call-template>
              </fo:table-header>

              <fo:table-body>
                <xsl:call-template name="generate-table-values">
                  <xsl:with-param name="columns" select="$columns"/>
                  <xsl:with-param name="counter" select="$scalars/InforceYear + 1"/>
                  <xsl:with-param name="max-counter" select="$max-lapse-year"/>
                  <xsl:with-param name="inforceyear" select="0 - $scalars/InforceYear"/>
                </xsl:call-template>
              </fo:table-body>
            </fo:table>
          </fo:block>
          <!-- endofdoc block id implemented as the "otherwise" condition in
               an xsl:choose instead of xsl:if !='1' so that the XML item
             'Supplemental Report' need not exist in the XML document for
             page numbering to work properly -->
          <xsl:if test="not($has_supplemental_report)">
            <fo:block id="endofdoc"/>
          </xsl:if>
        </fo:flow>
      </fo:page-sequence>

      <!-- Supplemental Report -->
      <!-- Body page -->
      <xsl:if test="$has_supplemental_report">
        <fo:page-sequence master-reference="supplemental-report">

          <!-- Define the contents of the header. -->
          <fo:static-content flow-name="xsl-region-before">
            <fo:block text-align="left">
              <xsl:call-template name="company-logo"/>
            </fo:block>
            <xsl:call-template name="standardheader"/>
            <fo:block text-align="center" font-size="10.0pt" space-before="3.0pt">
              <xsl:value-of select="illustration/supplementalreport/title"/>
            </fo:block>
            <xsl:call-template name="dollar-units"/>
          </fo:static-content>

          <!-- Define the contents of the footer. -->
          <xsl:call-template name="standardfooter">
            <xsl:with-param name="disclaimer">
              The Non-Guaranteed Values depicted above reflect an interest rate scale described in the Tabular Detail, and current scale monthly charges. These values
              are not guaranteed and depend upon company experience. Column headings indicate whether benefits and values are guaranteed or not guaranteed. This
              illustration assumes that non-guaranteed elements will continue unchanged for all years shown. This is not likely to occur and actual results may be
              more or less favorable than shown. Non-guaranteed elements are subject to change by the insurer. Factors that may affect future policy performance
              include the company's expectations for future mortality, investments, persistency, profits and expenses.
            </xsl:with-param>
          </xsl:call-template>

          <!-- Supplemental report body -->
          <xsl:call-template name="supplemental-report-body"/>
        </fo:page-sequence>
      </xsl:if>

      <xsl:if test="$scalars/IsInforce!='1'">
        <!-- Numeric Summary Attachment - (Only for New Business) -->
        <!-- Body page -->
        <fo:page-sequence master-reference="numeric-summary-attachment">

          <!-- Define the contents of the header. -->
          <fo:static-content flow-name="xsl-region-before">
            <fo:block text-align="left">
              <xsl:call-template name="company-logo"/>
            </fo:block>
            <xsl:call-template name="standardheader"/>
            <fo:block text-align="center" font-size="10.0pt">
              <xsl:text>Numeric Summary</xsl:text>
            </fo:block>
            <xsl:call-template name="dollar-units"/>
          </fo:static-content>

          <!-- Define the contents of the footer. -->
          <xsl:call-template name="standardfooter">
            <xsl:with-param name="omit-pagenumber" select="1"/>
          </xsl:call-template>

          <xsl:call-template name="numeric-summary-report"/>

        </fo:page-sequence>
      </xsl:if>
    </fo:root>
  </xsl:template>

  <xsl:template name="standardheader">
    <fo:block text-align="center" font-size="9.0pt" padding-bottom="1em">
      <xsl:choose>
        <xsl:when test="$scalars/IsInforce!='1'">
          <fo:block>
            <xsl:text>LIFE INSURANCE BASIC ILLUSTRATION</xsl:text>
          </fo:block>
        </xsl:when>
        <xsl:otherwise>
          <fo:block>
            <xsl:text>LIFE INSURANCE IN FORCE BASIC ILLUSTRATION</xsl:text>
          </fo:block>
        </xsl:otherwise>
      </xsl:choose>
      <fo:block>
        <xsl:value-of select="$scalars/InsCoName"/>
      </fo:block>
      <xsl:if test="$scalars/ProducerName!='0'">
        <fo:block>
          <xsl:text>Presented by: </xsl:text>
          <xsl:value-of select="$scalars/ProducerName"/>
        </fo:block>
      </xsl:if>
      <xsl:if test="$scalars/ProducerStreet!='0'">
        <fo:block>
          <xsl:value-of select="$scalars/ProducerStreet"/>
        </fo:block>
      </xsl:if>
      <xsl:if test="$scalars/ProducerCity!='0'">
        <fo:block>
          <xsl:value-of select="$scalars/ProducerCity"/>
        </fo:block>
      </xsl:if>
    </fo:block>
    <xsl:variable name="header-width" select="33"/>
    <xsl:variable name="header-field-width">
      <xsl:value-of select="$header-width * 0.44"/>
      <xsl:text>pc</xsl:text>
    </xsl:variable>
    <fo:block padding-before="5pt" font-size="9.0pt" font-family="sans-serif">
      <fo:table table-layout="fixed" width="100%">
        <fo:table-column column-width="125mm"/>
        <fo:table-column column-width="2mm"/>
        <fo:table-column column-width="proportional-column-width(1)"/>
        <fo:table-body>
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:text>Prepared for:</xsl:text>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell><fo:block/></fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:text>Initial </xsl:text>
                <xsl:if test="$scalars/HasTerm!='0'">
                  <xsl:text> Total</xsl:text>
                </xsl:if>
                <xsl:text> Face Amount: $</xsl:text>
                <xsl:value-of select="$scalars/InitTotalSA"/>
              </fo:block>
              <!-- Remove date prepared....now exists in footer
              <fo:block text-align="left">
                <xsl:text>Date Prepared: </xsl:text>
                <xsl:call-template name="date-prepared"/>
              </fo:block> -->
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:text>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Group Name: </xsl:text>
                <xsl:call-template name="limitstring">
                  <xsl:with-param name="passString" select="$scalars/CorpName"/>
                  <xsl:with-param name="length" select="50"/>
                </xsl:call-template>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell><fo:block/></fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:if test="$scalars/HasTerm!='0'">
                  <xsl:text>Initial Base Face Amount: $</xsl:text>
                  <xsl:value-of select="$scalars/InitBaseSpecAmt"/>
                </xsl:if>
              </fo:block>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <xsl:choose>
                <xsl:when test="$is_composite">
                  <fo:block text-align="left">
                    <xsl:text>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Composite Illustration</xsl:text>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="left">
                    <xsl:text>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Insured: </xsl:text>
                    <xsl:call-template name="limitstring">
                      <xsl:with-param name="passString" select="$scalars/Insured1"/>
                      <xsl:with-param name="length" select="50"/>
                    </xsl:call-template>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
            <fo:table-cell><fo:block/></fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:if test="$scalars/HasTerm!='0'">
                  <xsl:text>Initial Term Face Amount: $</xsl:text>
                  <xsl:value-of select="$scalars/InitTermSpecAmt"/>
                </xsl:if>
              </fo:block>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:if test="not($is_composite)">
                  <xsl:text>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Age: </xsl:text>
                  <xsl:value-of select="$scalars/Age"/>
                </xsl:if>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell><fo:block/></fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:text>Guaranteed Crediting Rate: </xsl:text>
                <xsl:value-of select="$scalars/InitAnnGenAcctInt_Guaranteed"/>
              </fo:block>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:text>Product: </xsl:text>
                <xsl:value-of select="$scalars/PolicyForm"/>
                <xsl:text> (</xsl:text>
                <xsl:value-of select="$scalars/PolicyMktgName"/>
                <xsl:text>)</xsl:text>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell><fo:block/></fo:table-cell>
            <xsl:choose>
              <xsl:when test="$scalars/InforceYear!=0">
                <fo:table-cell>
                  <fo:block text-align="left">
                    <xsl:text>Current Illustrated Crediting Rate: </xsl:text>
                    <xsl:call-template name="ultimate_interest_rate">
                      <xsl:with-param name="counter" select="$scalars/InforceYear + 1"/>
                    </xsl:call-template>
                  </fo:block>
                </fo:table-cell>
              </xsl:when>
              <xsl:otherwise>
                <fo:table-cell>
                  <fo:block text-align="left">
                    <xsl:text>Initial Illustrated Crediting Rate: </xsl:text>
                    <xsl:value-of select="$scalars/InitAnnGenAcctInt_Current"/>
                  </fo:block>
                </fo:table-cell>
              </xsl:otherwise>
            </xsl:choose>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$ModifiedSinglePremium='1'">
                  <fo:block text-align="left">
                    <xsl:text>Modified Single Premium Adjustable Life Insurance Policy</xsl:text>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="left">
                    <xsl:value-of select="$scalars/PolicyLegalName"/>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
            <fo:table-cell><fo:block/></fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="left">
                <!-- Single Premium Logic -->
                <xsl:if test="$SinglePremium='1' and $scalars/InforceYear &lt;= 4">
                  <xsl:text>Ultimate Illustrated Crediting Rate: </xsl:text>
                    <xsl:value-of select="$vectors[@name='AnnGAIntRate_Current']/duration[6]/@column_value"/>
                </xsl:if>
              </fo:block>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$SinglePremium!='1'">
                  <fo:block text-align="left">
                    <xsl:text>Initial Premium: </xsl:text>
                    <xsl:value-of select="$scalars/InitPrem"/>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="left">
                    <xsl:text>Single Premium: </xsl:text>
                    <xsl:value-of select="$scalars/InitPrem"/>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
            <fo:table-cell><fo:block/></fo:table-cell>
            <fo:table-cell>
              <!-- Update with "FriendlyUWType" - Get From Greg -->
              <!-- "&IF(UWType="Medical","Fully underwritten",
              IF(AND(State="TX",UWType="Guaranteed issue"),"Substandard *",UWType))) -->
              <fo:block text-align="left">
                <xsl:if test="not($is_composite)">
                  <xsl:choose>
                    <xsl:when test="$scalars/UWType='Medical'">
                      <xsl:text>Underwriting Type: Fully underwritten</xsl:text>
                    </xsl:when>
                    <xsl:when test="$scalars/StatePostalAbbrev='TX'">
                      <xsl:choose>
                        <xsl:when test="$scalars/UWType='Guaranteed issue'">
                          <xsl:text>Underwriting Type: Substandard *</xsl:text>
                        </xsl:when>
                        <xsl:otherwise>
                          <xsl:text>Underwriting Type: </xsl:text>
                          <xsl:value-of select="$scalars/UWType"/>
                        </xsl:otherwise>
                      </xsl:choose>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:text>Underwriting Type: </xsl:text>
                      <xsl:value-of select="$scalars/UWType"/>
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:if>
              </fo:block>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:choose>
                  <xsl:when test="$is_composite">
                    <xsl:if test="$scalars/Franchise!=''">
                      <xsl:text>Master contract: </xsl:text>
                      <xsl:call-template name="limitstring">
                        <xsl:with-param name="passString" select="$scalars/Franchise"/>
                        <xsl:with-param name="length" select="30"/>
                      </xsl:call-template>
                    </xsl:if>
                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:text>Initial Death Benefit Option: </xsl:text>
                    <xsl:value-of select="$scalars/DBOptInitInteger+1"/>
                  </xsl:otherwise>
                </xsl:choose>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell><fo:block/></fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:if test="not($is_composite)">
                  <xsl:text>Rate Classification: </xsl:text>
                  <xsl:value-of select="$scalars/UWClass"/>
                  <xsl:text>, </xsl:text>
                  <xsl:value-of select="$scalars/Smoker"/>
                  <xsl:text>, </xsl:text>
                  <xsl:value-of select="$scalars/Gender"/>
                </xsl:if>
              </fo:block>
            </fo:table-cell>
          </fo:table-row>
          <xsl:if test="not($is_composite)">
            <fo:table-row>
              <fo:table-cell>
                <fo:block text-align="left">
                  <xsl:choose>
                    <xsl:when test="$scalars/Franchise!='' and $scalars/PolicyNumber!=''">
                      <xsl:text>Master contract: </xsl:text>
                      <xsl:call-template name="limitstring">
                        <xsl:with-param name="passString" select="$scalars/Franchise"/>
                        <xsl:with-param name="length" select="15"/>
                      </xsl:call-template>
                      <xsl:text>&nbsp;&nbsp;&nbsp;Contract number: </xsl:text>
                      <xsl:call-template name="limitstring">
                        <xsl:with-param name="passString" select="$scalars/PolicyNumber"/>
                        <xsl:with-param name="length" select="15"/>
                      </xsl:call-template>
                    </xsl:when>
                    <xsl:when test="$scalars/Franchise!=''">
                      <xsl:text>Master contract: </xsl:text>
                      <xsl:call-template name="limitstring">
                        <xsl:with-param name="passString" select="$scalars/Franchise"/>
                        <xsl:with-param name="length" select="30"/>
                      </xsl:call-template>
                    </xsl:when>
                    <xsl:when test="$scalars/PolicyNumber!=''">
                      <xsl:text>Contract number: </xsl:text>
                      <xsl:call-template name="limitstring">
                        <xsl:with-param name="passString" select="$scalars/PolicyNumber"/>
                        <xsl:with-param name="length" select="30"/>
                      </xsl:call-template>
                    </xsl:when>
                  </xsl:choose>
                </fo:block>
              </fo:table-cell>
              <fo:table-cell><fo:block/></fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="left">
                  <xsl:if test="$scalars/UWClass='Rated'">
                    <xsl:text>&nbsp;&nbsp;&nbsp;Table Rating: </xsl:text>
                    <xsl:value-of select="$scalars/SubstandardTable"/>
                  </xsl:if>
                </fo:block>
              </fo:table-cell>
            </fo:table-row>
          </xsl:if>
        </fo:table-body>
      </fo:table>
    </fo:block>
  </xsl:template>

  <xsl:template name="numeric-summary-values">
    <xsl:param name="columns"/>
    <xsl:param name="counter"/>
    <xsl:param name="age70"/>
    <xsl:param name="prioryears"/>
    <xsl:variable name="PolicyYear_is_not_zero" select="$vectors[@name='PolicyYear']/duration[$counter]/@column_value!='0'"/>
    <fo:table-row>
      <xsl:for-each select="$columns">
        <fo:table-cell padding-top=".2pt" padding-bottom=".2pt">
          <xsl:if test="position()=1">
            <xsl:attribute name="padding-right">6pt</xsl:attribute>
          </xsl:if>
          <fo:block text-align="right">
            <xsl:choose>
              <xsl:when test="not(@name)">
              </xsl:when>
              <xsl:when test="(position() = 1) and ($age70 = 1)">
                Age 70
              </xsl:when>
              <xsl:when test="$PolicyYear_is_not_zero">
                <xsl:variable name="name" select="./@name"/>
                <xsl:value-of select="$vectors[@name=$name]/duration[$counter]/@column_value"/>
              </xsl:when>
              <xsl:when test="position() = 1">
                <xsl:value-of select="$counter"/>
              </xsl:when>
              <xsl:otherwise>
                0
              </xsl:otherwise>
            </xsl:choose>
          </fo:block>
        </fo:table-cell>
      </xsl:for-each>
    </fo:table-row>
    <!-- Display Only Summary Years -->
    <xsl:if test="$age70!=1">
      <xsl:if test="$prioryears!=1">
        <xsl:choose>
          <xsl:when test="$counter &lt; 30">
            <xsl:choose>
              <xsl:when test="$counter=5">
                <!-- Display lapse years that occur prior to year 10 -->
                <xsl:if test="$scalars/LapseYear_Guaranteed &lt; 9">
                  <xsl:if test="$scalars/LapseYear_Guaranteed &gt; 4">
                    <xsl:call-template name="numeric-summary-values">
                      <xsl:with-param name="columns" select="$columns"/>
                      <xsl:with-param name="counter" select="$scalars/LapseYear_Guaranteed + 1"/>
                      <xsl:with-param name="age70" select="0"/>
                      <xsl:with-param name="prioryears" select="1"/>
                    </xsl:call-template>
                  </xsl:if>
                </xsl:if>
                <xsl:if test="$scalars/LapseYear_Midpoint &lt; 9">
                  <xsl:if test="$scalars/LapseYear_Midpoint &gt; 4">
                    <xsl:if test="$scalars/LapseYear_Midpoint &gt; $scalars/LapseYear_Guaranteed">
                      <xsl:call-template name="numeric-summary-values">
                        <xsl:with-param name="columns" select="$columns"/>
                        <xsl:with-param name="counter" select="$scalars/LapseYear_Midpoint + 1"/>
                        <xsl:with-param name="age70" select="0"/>
                        <xsl:with-param name="prioryears" select="1"/>
                      </xsl:call-template>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <xsl:if test="$scalars/LapseYear_Current &lt; 9">
                  <xsl:if test="$scalars/LapseYear_Current &gt; 4">
                    <xsl:if test="$scalars/LapseYear_Current &gt; $scalars/LapseYear_Midpoint">
                      <xsl:call-template name="numeric-summary-values">
                        <xsl:with-param name="columns" select="$columns"/>
                        <xsl:with-param name="counter" select="$scalars/LapseYear_Current + 1"/>
                        <xsl:with-param name="age70" select="0"/>
                        <xsl:with-param name="prioryears" select="1"/>
                      </xsl:call-template>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <!-- Create year 10 values -->
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="columns" select="$columns"/>
                  <xsl:with-param name="counter" select="$counter + 5"/>
                  <xsl:with-param name="age70" select="0"/>
                </xsl:call-template>
              </xsl:when>
              <xsl:otherwise>
                <!-- Display lapse years that occur prior to next display year -->
                <xsl:if test="$scalars/LapseYear_Guaranteed &lt; $counter + 9">
                  <xsl:if test="$scalars/LapseYear_Guaranteed &lt; $scalars/MaxDuration">
                    <xsl:if test="$scalars/LapseYear_Guaranteed &gt; $counter - 1">
                      <xsl:call-template name="numeric-summary-values">
                        <xsl:with-param name="columns" select="$columns"/>
                        <xsl:with-param name="counter" select="$scalars/LapseYear_Guaranteed + 1"/>
                        <xsl:with-param name="age70" select="0"/>
                        <xsl:with-param name="prioryears" select="1"/>
                      </xsl:call-template>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <xsl:if test="$scalars/LapseYear_Midpoint &lt; $counter + 9">
                  <xsl:if test="$scalars/LapseYear_Midpoint &lt; $scalars/MaxDuration">
                    <xsl:if test="$scalars/LapseYear_Midpoint &gt; $counter - 1">
                      <xsl:if test="$scalars/LapseYear_Midpoint &gt; $scalars/LapseYear_Guaranteed">
                        <xsl:call-template name="numeric-summary-values">
                          <xsl:with-param name="columns" select="$columns"/>
                          <xsl:with-param name="counter" select="$scalars/LapseYear_Midpoint + 1"/>
                          <xsl:with-param name="age70" select="0"/>
                          <xsl:with-param name="prioryears" select="1"/>
                        </xsl:call-template>
                      </xsl:if>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <xsl:if test="$scalars/LapseYear_Current &lt; $counter + 9">
                  <xsl:if test="$scalars/LapseYear_Current &lt; $scalars/MaxDuration">
                    <xsl:if test="$scalars/LapseYear_Current &gt; $counter - 1">
                      <xsl:if test="$scalars/LapseYear_Current &gt; $scalars/LapseYear_Midpoint">
                        <xsl:call-template name="numeric-summary-values">
                          <xsl:with-param name="columns" select="$columns"/>
                          <xsl:with-param name="counter" select="$scalars/LapseYear_Current + 1"/>
                          <xsl:with-param name="age70" select="0"/>
                          <xsl:with-param name="prioryears" select="1"/>
                        </xsl:call-template>
                      </xsl:if>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <!-- Create year 20 and 30 values -->
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="columns" select="$columns"/>
                  <xsl:with-param name="counter" select="$counter + 10"/>
                  <xsl:with-param name="age70" select="0"/>
                </xsl:call-template>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:when>
          <xsl:otherwise>
            <xsl:if test="$scalars/LapseYear_Guaranteed &lt; $scalars/MaxDuration">
              <!-- Display lapse years that occur prior to next display year -->
              <xsl:if test="$scalars/LapseYear_Guaranteed &gt; $counter - 1">
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="columns" select="$columns"/>
                  <xsl:with-param name="counter" select="$scalars/LapseYear_Guaranteed + 1"/>
                  <xsl:with-param name="age70" select="0"/>
                  <xsl:with-param name="prioryears" select="1"/>
                </xsl:call-template>
              </xsl:if>
            </xsl:if>
            <xsl:if test="$scalars/LapseYear_Midpoint &lt; $scalars/MaxDuration">
              <xsl:if test="$scalars/LapseYear_Midpoint &gt; $counter - 1">
                <xsl:if test="$scalars/LapseYear_Midpoint &gt; $scalars/LapseYear_Guaranteed">
                  <xsl:call-template name="numeric-summary-values">
                    <xsl:with-param name="columns" select="$columns"/>
                    <xsl:with-param name="counter" select="$scalars/LapseYear_Midpoint + 1"/>
                    <xsl:with-param name="age70" select="0"/>
                    <xsl:with-param name="prioryears" select="1"/>
                  </xsl:call-template>
                </xsl:if>
              </xsl:if>
            </xsl:if>
            <xsl:if test="$scalars/LapseYear_Current &lt; $scalars/MaxDuration">
              <xsl:if test="$scalars/LapseYear_Current &gt; $counter - 1">
                <xsl:if test="$scalars/LapseYear_Current &gt; $scalars/LapseYear_Midpoint">
                  <xsl:call-template name="numeric-summary-values">
                    <xsl:with-param name="columns" select="$columns"/>
                    <xsl:with-param name="counter" select="$scalars/LapseYear_Current + 1"/>
                    <xsl:with-param name="age70" select="0"/>
                    <xsl:with-param name="prioryears" select="1"/>
                  </xsl:call-template>
                </xsl:if>
              </xsl:if>
            </xsl:if>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template name="numeric-summary-report">
    <xsl:variable name="numeric-summary-report-columns">
      <column name="PolicyYear">                               |               |Policy _Year    </column>
      <column name="GrossPmt">                                 |               |Premium _Outlay </column>
      <column name="AcctVal_Guaranteed">    Guaranteed Values  |               |Account _Value  </column>
      <column name="CSVNet_Guaranteed">     Guaranteed Values  |               |Cash Surr _Value</column>
      <column name="EOYDeathBft_Guaranteed">Guaranteed Values  |               |Death _Benefit  </column>
      <column/>
      <column name="AcctVal_Midpoint">    Non-Guaranteed Values|Midpoint Values|Account _Value  </column>
      <column name="CSVNet_Midpoint">     Non-Guaranteed Values|Midpoint Values|Cash Surr _Value</column>
      <column name="EOYDeathBft_Midpoint">Non-Guaranteed Values|Midpoint Values|Death _Benefit  </column>
      <column>                            Non-Guaranteed Values|</column>
      <column name="AcctVal_Current">     Non-Guaranteed Values| Current Values|Account _Value  </column>
      <column name="CSVNet_Current">      Non-Guaranteed Values| Current Values|Cash Surr _Value</column>
      <column name="EOYDeathBft_Current"> Non-Guaranteed Values| Current Values|Death _Benefit  </column>
    </xsl:variable>
    <xsl:variable name="columns" select="document('')//xsl:variable[@name='numeric-summary-report-columns']/column"/>

    <!-- The main contents of the body page -->
    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9.0pt" font-family="serif">
        <fo:table table-layout="fixed" width="100%">
          <xsl:call-template name="generate-table-columns">
            <xsl:with-param name="columns" select="$columns"/>
          </xsl:call-template>

          <fo:table-header>
            <xsl:call-template name="generate-table-headers">
              <xsl:with-param name="columns" select="$columns"/>
            </xsl:call-template>
          </fo:table-header>

          <!-- Create Numeric Summary Values -->
          <fo:table-body>
              <!-- Display summary values if policy lapses prior to year 5 -->
            <xsl:if test="$scalars/LapseYear_Guaranteed &lt; 4">
              <xsl:call-template name="numeric-summary-values">
                <xsl:with-param name="columns" select="$columns"/>
                <xsl:with-param name="counter" select="$scalars/LapseYear_Guaranteed + 1"/>
                <xsl:with-param name="age70" select="0"/>
                <xsl:with-param name="prioryears" select="1"/>
              </xsl:call-template>
            </xsl:if>
            <xsl:if test="$scalars/LapseYear_Midpoint &lt; 4">
              <xsl:if test="$scalars/LapseYear_Midpoint &gt; $scalars/LapseYear_Guaranteed">
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="columns" select="$columns"/>
                  <xsl:with-param name="counter" select="$scalars/LapseYear_Midpoint + 1"/>
                  <xsl:with-param name="age70" select="0"/>
                  <xsl:with-param name="prioryears" select="1"/>
                </xsl:call-template>
              </xsl:if>
            </xsl:if>
            <xsl:if test="$scalars/LapseYear_Current &lt; 4">
              <xsl:if test="$scalars/LapseYear_Current &gt; $scalars/LapseYear_Midpoint">
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="columns" select="$columns"/>
                  <xsl:with-param name="counter" select="$scalars/LapseYear_Current + 1"/>
                  <xsl:with-param name="age70" select="0"/>
                  <xsl:with-param name="prioryears" select="1"/>
                </xsl:call-template>
              </xsl:if>
            </xsl:if>
            <xsl:call-template name="numeric-summary-values">
              <xsl:with-param name="columns" select="$columns"/>
              <xsl:with-param name="counter" select="5"/>
              <xsl:with-param name="age70" select="0"/>
            </xsl:call-template>
            <xsl:if test="not($is_composite)">
              <xsl:if test="$scalars/Age &lt; 70">
                <fo:table-row>
                  <fo:table-cell padding="8pt">
                    <fo:block/>
                  </fo:table-cell>
                </fo:table-row>
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="columns" select="$columns"/>
                  <xsl:with-param name="counter" select="70 - $scalars/Age"/>
                  <xsl:with-param name="age70" select="1"/>
                </xsl:call-template>
              </xsl:if>
            </xsl:if>
          </fo:table-body>
        </fo:table>
      </fo:block>
      <xsl:choose>
        <xsl:when test="$is_composite">
          <fo:block text-align="left" font-size="9.0pt" padding-top="2em">
            <xsl:text>The year of policy lapse on a guaranteed, midpoint and current basis is not depicted in the above table of values for this composite illustration because it is not applicable on a case basis.</xsl:text>
          </fo:block>
        </xsl:when>
        <xsl:when test="$scalars/LapseYear_Guaranteed &lt; $scalars/MaxDuration">
          <fo:block text-align="left" font-size="9.0pt" padding-top="2em">
            <xsl:text>Additional premium will be required in year </xsl:text>
            <xsl:value-of select="$scalars/LapseYear_Guaranteed+1"/>
            <xsl:text> or contract will lapse based on guaranteed monthly charges and interest rate.</xsl:text>
          </fo:block>
          <xsl:if test="$scalars/LapseYear_Midpoint &lt; $scalars/MaxDuration">
            <fo:block text-align="left" font-size="9.0pt">
              <xsl:text>Additional premium will be required in year </xsl:text>
              <xsl:value-of select="$scalars/LapseYear_Midpoint+1"/>
              <xsl:text> or contract will lapse based on midpoint monthly charges and interest rate.</xsl:text>
            </fo:block>
          </xsl:if>
          <xsl:if test="$scalars/LapseYear_Current &lt; $scalars/MaxDuration">
            <fo:block text-align="left" font-size="9.0pt">
              <xsl:text>Additional premium will be required in year </xsl:text>
              <xsl:value-of select="$scalars/LapseYear_Current+1"/>
              <xsl:text> or contract will lapse based on current monthly charges and interest rate.</xsl:text>
            </fo:block>
          </xsl:if>
        </xsl:when>
      </xsl:choose>
      <xsl:if test="$scalars/IsMec='1'">
        <fo:block text-align="left" font-size="9.0pt" padding-top="1em">
          <xsl:text>IMPORTANT TAX DISCLOSURE: This is a Modified Endowment Contract. Please refer to the Narrative Summary for additional information.</xsl:text>
        </fo:block>
      </xsl:if>
      <fo:block text-align="center" font-size="9.0pt" padding-top="2em">
        <xsl:text>Certification Statements</xsl:text>
      </fo:block>
      <fo:block text-align="left" font-size="9.0pt" padding-top="1em">
        <xsl:text>CONTRACT OWNER / APPLICANT</xsl:text>
      </fo:block>
      <xsl:if test="$scalars/InterestDisclaimer!=''">
        <fo:block text-align="left" font-size="9.0pt" padding-bottom="1em">
          <xsl:text>I understand that at the present time higher current interest rates are credited for policies with case premiums in the amount of </xsl:text>
          <xsl:value-of select="$scalars/InterestDisclaimer"/>
        </fo:block>
      </xsl:if>
      <xsl:choose>
        <xsl:when test="$scalars/StatePostalAbbrev='IL'">
          <fo:block text-align="left" font-size="9.0pt">
            <xsl:text>I have received a copy of this illustration and understand that this illustration assumes that the currently illustrated non-guaranteed elements will continue unchanged for all years shown. This is not likely to occur, and actual results may be more or less favorable than those shown.</xsl:text>
          </fo:block>
        </xsl:when>
        <xsl:otherwise>
          <xsl:choose>
            <xsl:when test="$scalars/StatePostalAbbrev='TX'">
              <fo:block text-align="left" font-size="9.0pt">
                <xsl:text>A copy of this illustration has been provided to the Applicant/Policyowner. </xsl:text>
              </fo:block>
            </xsl:when>
            <xsl:otherwise>
              <fo:block text-align="left" font-size="9.0pt">
                <xsl:text>I have received a copy of this illustration, and I understand that any non-guaranteed elements illustrated are subject to change and could be either higher or lower.  Additionally, I have been informed by my agent that these values are not guaranteed.</xsl:text>
              </fo:block>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:otherwise>
      </xsl:choose>
      <fo:block text-align="left" font-size="9.0pt" text-decoration="overline" padding-top="2em">
        CONTRACT OWNER OR APPLICANT SIGNATURE &nbsp;&nbsp;&nbsp;
        <fo:inline text-decoration="no-overline">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</fo:inline>DATE &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
      </fo:block>
      <fo:block text-align="left" font-size="9.0pt" padding-top="2em">
        <xsl:text>AGENT / AUTHORIZED REPRESENTATIVE</xsl:text>
      </fo:block>
      <xsl:choose>
        <xsl:when test="$scalars/StatePostalAbbrev='IL'">
          <fo:block text-align="left" font-size="9.0pt">
            <xsl:text>I have informed the applicant or policyowner that this illustration assumes that the currently illustrated non-guaranted elements will continue unchanged for all years shown. This is not likely to occur, and actual results may be more or less favorable than those shown.</xsl:text>
          </fo:block>
        </xsl:when>
        <xsl:otherwise>
          <xsl:choose>
            <xsl:when test="$scalars/StatePostalAbbrev='TX'">
              <fo:block text-align="left" font-size="9.0pt">
                <xsl:text>A copy of this illustration has been provided to the Applicant/Policyowner.</xsl:text>
              </fo:block>
            </xsl:when>
            <xsl:otherwise>
              <fo:block text-align="left" font-size="9.0pt">
                <xsl:text>I certify that this illustration has been presented to the applicant, and that I have explained that any non-guaranteed elements illustrated are subject to change.  I have made no statements that are inconsistent with the illustration.</xsl:text>
              </fo:block>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:otherwise>
      </xsl:choose>
      <fo:block text-align="left" font-size="9.0pt" text-decoration="overline" padding-top="2em">
        AGENT OR AUTHORIZED REPRESENTATIVE &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
        <fo:inline text-decoration="no-overline">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</fo:inline>DATE &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
      </fo:block>
    </fo:flow>
  </xsl:template>

  <xsl:template name="removeamps">
    <xsl:param name="title"/>
    <xsl:if test="contains($title, '&amp;')">
      <xsl:call-template name="concatenate">
        <xsl:with-param name="textbefore" select="substring-before($title,'amp;')"/>
        <xsl:with-param name="textafter" select="substring-after($title,'amp;')"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="concatenate">
    <xsl:param name="textbefore"/>
    <xsl:param name="textafter"/>
    <xsl:call-template name="removeamps">
      <xsl:with-param name="title" select="concat($textbefore,$textafter)"/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="standardfooter">
    <xsl:param name="omit-pagenumber" select="boolean(0)"/>
    <xsl:param name="disclaimer" select="string('')"/>
    <xsl:call-template name="generic-footer">
      <xsl:with-param name="top-block" select="$disclaimer"/>
      <xsl:with-param name="left-block">
        <fo:block>
          Date Prepared:
          <xsl:call-template name="date-prepared"/>
        </fo:block>
        <!-- Version Number -->
        <xsl:if test="$scalars/LmiVersion!=''">
          <fo:block>
            System Version:
            <xsl:value-of select="$scalars/LmiVersion"/>
          </fo:block>
        </xsl:if>
      </xsl:with-param>
      <xsl:with-param name="center-block">
        <xsl:choose>
          <xsl:when test="$omit-pagenumber">
            Attachment
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="page-of"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:with-param>
      <xsl:with-param name="right-block">
        <!--
        APACHE !! The '/text()' suffix in the XPath expression is a workaround
        for Apache-FOP 0.20.5 bug that somehow treats <xsl:value-of/> as
        <xsl:copy-of/> if it is encountered inside a <xsl:with-param/> as
        a single child. Therefore we explicitly convert the expression into
        a string.
        -->
        <xsl:value-of select="$scalars/InsCoName/text()"/>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="ultimate_interest_rate">
    <xsl:param name="counter"/>
    <xsl:value-of select="$vectors[@name='AnnGAIntRate_Current']/duration[$counter]/@column_value"/>
  </xsl:template>

  <xsl:template name="set_single_premium">
    <xsl:choose>
      <xsl:when test="string-length($scalars/PolicyMktgName) &gt; 5">
        <xsl:choose>
          <xsl:when test="substring($scalars/PolicyLegalName, 1, 6) !='Single'">
            <xsl:text>0</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>1</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>0</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="set_modified_single_premium">
    <xsl:choose>
      <xsl:when test="string-length($scalars/PolicyMktgName) &gt; 5">
        <xsl:choose>
          <xsl:when test="substring($scalars/PolicyLegalName, 1, 6) !='Single'">
            <xsl:text>0</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="$scalars/StatePostalAbbrev='MA'">
                <xsl:text>1</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text>0</xsl:text>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>0</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="set_group_experience_rating">
    <xsl:choose>
      <xsl:when test="$scalars/PolicyLegalName='Group Flexible Premium Adjustable Life Insurance Policy'">
        <xsl:text>1</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>0</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>
