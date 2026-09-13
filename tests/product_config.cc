#include "product_config.h"

namespace ondewo_client_test {

// Mirrors the #include list of the generated public-api.h, one .proto per pair of headers:
//   sed -n 's|^#include "\(.*\)\.pb\.h"$|\1|p' public-api.h | sed 's|\.grpc$||' | sort -u
//
// CSI is a composition product: ondewo/csi/conversation.proto is the only .proto the CSI API
// declares itself, and it pulls the whole NLU, S2T and T2S surface in as imports - a CSI
// client that could not also speak those is useless, so all of them are generated and linked.
const std::vector<std::string> kProtoFileNames = {
    "google/api/annotations.proto",
    "google/api/http.proto",
    "google/rpc/status.proto",
    "google/type/latlng.proto",
    "ondewo/csi/conversation.proto",
    "ondewo/nlu/agent.proto",
    "ondewo/nlu/aiservices.proto",
    "ondewo/nlu/ccai_project.proto",
    "ondewo/nlu/common.proto",
    "ondewo/nlu/context.proto",
    "ondewo/nlu/entity_type.proto",
    "ondewo/nlu/intent.proto",
    "ondewo/nlu/llm_evaluation.proto",
    "ondewo/nlu/operation_metadata.proto",
    "ondewo/nlu/operations.proto",
    "ondewo/nlu/project_role.proto",
    "ondewo/nlu/project_statistics.proto",
    "ondewo/nlu/rag.proto",
    "ondewo/nlu/server_statistics.proto",
    "ondewo/nlu/session.proto",
    "ondewo/nlu/user.proto",
    "ondewo/nlu/utility.proto",
    "ondewo/nlu/webhook.proto",
    "ondewo/s2t/speech-to-text.proto",
    "ondewo/t2s/text-to-speech.proto",
};

// The product's own service plus the 16 NLU services and the S2T / T2S services it re-exports.
// There is no ondewo.qa.QA here - CSI does not import ondewo/qa/qa.proto.
const std::vector<std::string> kServiceFullNames = {
    "ondewo.csi.Conversations",
    "ondewo.nlu.Agents",
    "ondewo.nlu.AiServices",
    "ondewo.nlu.CcaiProjects",
    "ondewo.nlu.Contexts",
    "ondewo.nlu.EntityTypes",
    "ondewo.nlu.Intents",
    "ondewo.nlu.LlmEvaluations",
    "ondewo.nlu.Operations",
    "ondewo.nlu.ProjectRoles",
    "ondewo.nlu.ProjectStatistics",
    "ondewo.nlu.Rags",
    "ondewo.nlu.ServerStatistics",
    "ondewo.nlu.Sessions",
    "ondewo.nlu.Users",
    "ondewo.nlu.Utilities",
    "ondewo.nlu.Webhook",
    "ondewo.s2t.Speech2Text",
    "ondewo.t2s.Text2Speech",
};

const std::vector<ExpectedMethod> kExpectedMethods = {
    // The complete pipeline CRUD surface of the product's own service ...
    {"ondewo.csi.Conversations", "CreateS2sPipeline"},
    {"ondewo.csi.Conversations", "GetS2sPipeline"},
    {"ondewo.csi.Conversations", "UpdateS2sPipeline"},
    {"ondewo.csi.Conversations", "DeleteS2sPipeline"},
    {"ondewo.csi.Conversations", "ListS2sPipelines"},
    // ... the RPC the product exists for, a bidirectional audio stream, plus the
    // server-streaming control channel that rides alongside it ...
    {"ondewo.csi.Conversations", "S2sStream"},
    {"ondewo.csi.Conversations", "GetControlStream"},
    {"ondewo.csi.Conversations", "SetControlStatus"},
    {"ondewo.csi.Conversations", "CheckUpstreamHealth"},
    // ... and one RPC from each of the three upstream packages the same library carries, so a
    // .proto that silently stops being generated cannot go unnoticed.
    {"ondewo.s2t.Speech2Text", "TranscribeStream"},
    {"ondewo.t2s.Text2Speech", "Synthesize"},
    {"ondewo.nlu.Sessions", "DetectIntent"},
};

const std::string kScalarMessageFullName = "ondewo.csi.S2sPipeline";

const std::string kEnumFullName = "ondewo.csi.ControlStatus";

// ONDEWO CSI API 5.5.0 generates 846 messages (map entries excluded), 98 enums and 2542
// singular scalar fields across the files listed above. The floors sit just below that.
const int kMinimumMessageCount = 830;
const int kMinimumEnumCount = 95;
const int kMinimumScalarFieldCount = 2500;

}  // namespace ondewo_client_test
