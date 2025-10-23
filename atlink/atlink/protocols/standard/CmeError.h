//
//  This file is part of ATLink.
//
//  ATLink is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ATLink is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with ATLink.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once

#include "atlink/core/Enum.h"
#include "atlink/core/Response.h"
#include "atlink/utils/EnumStringConverter.h"

namespace ATL_NS {
namespace Proto {
namespace Std {

class CmeError : public Core::AResponse {

  public:
    enum class Code {
        PhoneFailure = 0,
        NoConnection = 1,
        LinkReserved = 2,
        NotAllowed = 3,
        NotSupported = 4,
        PhSimPin = 5,
        PhFsimPin = 6,
        PhFsimPuk = 7,
        SimNotInserted = 10,
        SimPin = 11,
        SimPuk = 12,
        SimFailure = 13,
        SimBusy = 14,
        SimWrong = 15,
        IncorrectPassword = 16,
        SimPin2 = 17,
        SimPuk2 = 18,
        MemoryFull = 20,
        InvalidIndex = 21,
        NotFound = 22,
        MemoryFailure = 23,
        TextTooLong = 24,
        InvalidChars = 25,
        DialStringTooLong = 26,
        DialStringInvalid = 27,
        NoNetwork = 30,
        NetworkTimeout = 31,
        NetworkNotAllowed = 32,
        NetworkPin = 40,
        NetworkPuk = 41,
        NetworkSubsetPin = 42,
        NetworkSubsetPuk = 43,
        ServicePin = 44,
        ServicePuk = 45,
        CorpPin = 46,
        CorpPuk = 47,
        HiddenKeyRequired = 48,
        EapMethodNotSupported = 49,
        IncorrectParameters = 50,
        CommandDisabled = 51,
        CommandAborted = 52,
        NotAttachedRestricted = 53,
        NotAllowedEmergencyOnly = 54,
        NotAllowedRestricted = 55,
        FixedDialNumberOnly = 56,
        TemporarilyOutOfService = 57,
        LanguageOrAlphabetNotSupported = 58,
        UnexpectedDataValue = 59,
        SystemFailure = 60,
        DataMissing = 61,
        CallBarred = 62,
        MessageWaitingIndicationSubscriptionFailure = 63,
        Unknown = 100,
        ImsiUnknownInHss = 102,
        IllegalUe = 103,
        ImsiUnknownInVlr = 104,
        ImeiNotAccepted = 105,
        IllegalMe = 106,
        PsServicesNotAllowed = 107,
        PsAndNonPsServicesNotAllowed = 108,
        UeIdentityNotDerivedFromNetwork = 109,
        ImplicitlyDetached = 110,
        PlmnNotAllowed = 111,
        AreaNotAllowed = 112,
        RoamingNotAllowedInArea = 113,
        PsServicesNotAllowedInPlmn = 114,
        NoCellsInArea = 115,
        MscTemporarilyNotReachable = 116,
        NetworkFailureAttach = 117,
        CsDomainUnavailable = 118,
        EsmFailure = 119,
        Congestion = 122,
        MbmsBearerCapabilitiesInsufficientForService = 124,
        NotAuthorizedForCsg = 125,
        InsufficientResources = 126,
        MissingOrUnknownApn = 127,
        UnknownPdpAddressOrType = 128,
        UserAuthenticationFailed = 129,
        ActivationRejectedByGgsnOrGw = 130,
        ActivationRejectedUnspecified = 131,
        ServiceOptionNotSupported = 132,
        ServiceOptionNotSubscribed = 133,
        ServiceOptionOutOfOrder = 134,
        NsapiOrPtiAlreadyInUse = 135,
        RegularDeactivation = 136,
        QosNotAccepted = 137,
        CallCannotBeIdentified = 138,
        CsServiceTemporarilyUnavailable = 139,
        FeatureNotSupported = 140,
        SemanticErrorInTftOperation = 141,
        SyntacticalErrorInTftOperation = 142,
        UnknownPdpContext = 143,
        SemanticErrorsInPacketFilter = 144,
        SyntacticalErrorInPacketFilter = 145,
        PdpContextWithoutTftAlreadyActivated = 146,
        MulticastGroupMembershipTimeout = 147,
        GprsUnknown = 148,
        PdpAuthFailure = 149,
        InvalidMobileClass = 150,
        LastPdnDisconnectionNotAllowedLegacy = 151,
        LastPdnDisconnectionNotAllowed = 171,
        SemanticallyIncorrectMessage = 172,
        InvalidMandatoryInformation = 173,
        MessageTypeNotImplemented = 174,
        ConditionalIeError = 175,
        UnspecifiedProtocolError = 176,
        OperatorDeterminedBarring = 177,
        MaximumNumberOfBearersReached = 178,
        RequestedApnNotSupported = 179,
        RequestRejectedBcmViolation = 180,
        UnsupportedQciOr5QiValue = 181,
        UserDataViaControlPlaneCongested = 182,
        SmsProvidedViaGprsInRoutingArea = 183,
        InvalidPtiValue = 184,
        NoBearerActivated = 185,
        MessageNotCompatibleWithProtocolState = 186,
        RecoveryOnTimerExpiry = 187,
        InvalidTransactionIdValue = 188,
        ServiceOptionNotAuthorizedInPlmn = 189,
        NetworkFailureActivation = 190,
        ReactivationRequested = 191,
        Ipv4OnlyAllowed = 192,
        Ipv6OnlyAllowed = 193,
        SingleAddressBearersOnlyAllowed = 194,
        CollisionWithNetworkInitiatedRequest = 195,
        Ipv4V6OnlyAllowed = 196,
        NonIpOnlyAllowed = 197,
        BearerHandlingUnsupported = 198,
        ApnRestrictionIncompatible = 199,
        MultipleAccessToPdnConnectionNotAllowed = 200,
        EsmInformationNotReceived = 201,
        PdnConnectionNonexistent = 202,
        MultiplePdnConnectionSameApnNotAllowed = 203,
        SevereNetworkFailure = 204,
        InsufficientResourcesForSliceAndDnn = 205,
        UnsupportedSscMode = 206,
        InsufficientResourcesForSlice = 207,
        MessageTypeNotCompatibleWithProtocolState = 208,
        IeNotImplemented = 209,
        N1ModeNotAllowed = 210,
        RestrictedServiceArea = 211,
        LadnUnavailable = 212,
        MissingOrUnknownDnnInSlice = 213,
        NgksiAlreadyInUse = 214,
        PayloadNotForwarded = 215,
        Non3GppAccessTo5GcnNotAllowed = 216,
        ServingNetworkNotAuthorized = 217,
        DnnNotSupportedInSlice = 218,
        InsufficientUserPlaneResourcesForPduSessio = 219,
        OutOfLadnServiceArea = 220,
        PtiMismatch = 221,
        MaxDataRateForUserPlaneIntegrityTooLow = 222,
        SemanticErrorInQosOperation = 223,
        SyntacticalErrorInQosOperation = 224,
        InvalidMappedEpsBearerIdentity = 225,
        RedirectionTo5GcnRequired = 226,
        RedirectionToEpcRequired = 227,
        TemporarilyUnauthorizedForSnpn = 228,
        PermanentlyUnauthorizedForSnpn = 229,
        EthernetOnlyAllowed = 230,
        UnauthorizedForCag = 231,
        NoNetworkSlicesAvailable = 232,
        WirelineAccessAreaNotAllowed = 233,
    };

    Core::Enum<Code> code{};

    CmeError() : Core::AResponse("+CME ERROR:") {}
    ~CmeError() = default;
    void accept(Core::AInputVisitor &visitor) override {
        APacket::accept(visitor, code);
    }
};

} // namespace Std
} // namespace Proto
} // namespace ATL_NS
