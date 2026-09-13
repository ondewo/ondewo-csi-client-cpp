// Assertions against the concrete C++ types the CSI stubs generate.
//
// This is the per-product half of the suite: it names ondewo::csi types (and the ondewo::s2t /
// ondewo::t2s types the CSI library carries with them), so replicating the suite to another
// ONDEWO client means rewriting this file against that product's messages and services.
// Everything generic lives in test_generated_stubs.cc.

#include <chrono>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>

#include "ondewo/csi/conversation.grpc.pb.h"
#include "ondewo/csi/conversation.pb.h"
#include "ondewo/nlu/session.grpc.pb.h"
#include "ondewo/s2t/speech-to-text.grpc.pb.h"
#include "ondewo/s2t/speech-to-text.pb.h"
#include "ondewo/t2s/text-to-speech.grpc.pb.h"

namespace ondewo_client_test {
namespace {

// A channel to a port nothing listens on. gRPC connects lazily, so constructing stubs
// against it touches no network at all; the one test that does issue an RPC gives it a
// short deadline and asserts only that the call comes back as a failure.
std::shared_ptr<grpc::Channel> DeadChannel() {
  return grpc::CreateChannel("127.0.0.1:1", grpc::InsecureChannelCredentials());
}

TEST(TypedApi, MessageSurvivesSerializeAndParse) {
  ondewo::csi::S2sPipeline original;
  original.set_id("my-s2s-pipeline");
  original.set_s2t_pipeline_id("german_general");
  original.set_nlu_project_id("ae33586b-x2s2-494a-aa73-1af0589cfc56");
  original.set_nlu_language_code("de");
  original.set_t2s_pipeline_id("kerstin");

  std::string bytes;
  ASSERT_TRUE(original.SerializeToString(&bytes));
  EXPECT_FALSE(bytes.empty());

  ondewo::csi::S2sPipeline parsed;
  ASSERT_TRUE(parsed.ParseFromString(bytes));

  EXPECT_EQ(parsed.id(), "my-s2s-pipeline");
  EXPECT_EQ(parsed.s2t_pipeline_id(), "german_general");
  EXPECT_EQ(parsed.nlu_project_id(), "ae33586b-x2s2-494a-aa73-1af0589cfc56");
  EXPECT_EQ(parsed.nlu_language_code(), "de");
  EXPECT_EQ(parsed.t2s_pipeline_id(), "kerstin");
  EXPECT_EQ(parsed.SerializeAsString(), bytes);
}

// A response carrying a oneof alternative: the CSI stream multiplexes NLU, T2S and SIP
// payloads through one message, so which alternative is set has to survive the wire.
TEST(TypedApi, StreamResponseOneofSurvivesTheWire) {
  ondewo::csi::S2sStreamResponse original;
  original.mutable_sip_trigger()->set_type(ondewo::csi::SipTrigger::HUMAN_HANDOVER);
  original.set_utterance_id("utterance-1");
  original.set_chunk_index(3);
  original.set_last_chunk(true);
  original.set_turn_epoch(42);

  ondewo::csi::S2sStreamResponse parsed;
  ASSERT_TRUE(parsed.ParseFromString(original.SerializeAsString()));

  EXPECT_EQ(parsed.response_case(), ondewo::csi::S2sStreamResponse::kSipTrigger);
  EXPECT_EQ(parsed.sip_trigger().type(), ondewo::csi::SipTrigger::HUMAN_HANDOVER);
  EXPECT_EQ(parsed.utterance_id(), "utterance-1");
  EXPECT_EQ(parsed.chunk_index(), 3);
  EXPECT_TRUE(parsed.last_chunk());
  EXPECT_EQ(parsed.turn_epoch(), 42u);
}

// `optional string language = 9` of the S2T transcription config has proto3 explicit
// presence. Set to "" - the type's default - it must still reach the wire and still read back
// as *present*; a generator that drops the presence bit makes "" unsendable, which is exactly
// the class of bug that hit the Angular client.
TEST(TypedApi, ExplicitPresenceFieldSurvivesItsZeroValue) {
  ondewo::s2t::TranscribeRequestConfig original;
  EXPECT_FALSE(original.has_language());

  original.set_language("");
  ASSERT_TRUE(original.has_language());

  const std::string bytes = original.SerializeAsString();
  EXPECT_FALSE(bytes.empty()) << "an explicitly present \"\" was not written to the wire";

  ondewo::s2t::TranscribeRequestConfig parsed;
  ASSERT_TRUE(parsed.ParseFromString(bytes));
  EXPECT_TRUE(parsed.has_language()) << "presence of an empty value was lost on the wire";
  EXPECT_EQ(parsed.language(), "");

  original.clear_language();
  EXPECT_FALSE(original.has_language());
  EXPECT_TRUE(original.SerializeAsString().empty());
}

// A plain (non-optional) proto3 scalar has the opposite contract: its zero value is the
// default and must NOT be written. Asserting both directions is what proves the two field
// kinds really are generated differently.
TEST(TypedApi, PlainScalarZeroValueStaysOffTheWire) {
  ondewo::s2t::TranscribeRequestConfig config;
  config.set_s2t_pipeline_id("");
  EXPECT_TRUE(config.SerializeAsString().empty());

  config.set_s2t_pipeline_id("german_general");
  EXPECT_FALSE(config.SerializeAsString().empty());
}

TEST(TypedApi, EnumZeroValueIsTheUnspecifiedOne) {
  EXPECT_EQ(static_cast<int>(ondewo::csi::ControlStatus::OK), 0);
  EXPECT_EQ(ondewo::csi::ControlStatus_Name(ondewo::csi::ControlStatus::OK), "OK");
  EXPECT_EQ(static_cast<int>(ondewo::csi::SipTrigger::UNSPECIFIED), 0);
  EXPECT_EQ(static_cast<int>(ondewo::csi::ControlMessageServiceName::UNKNOWNNAME), 0);

  ondewo::csi::ControlStatus parsed = ondewo::csi::ControlStatus::EMERGENCY_STOP;
  ASSERT_TRUE(ondewo::csi::ControlStatus_Parse("OK", &parsed));
  EXPECT_EQ(parsed, ondewo::csi::ControlStatus::OK);

  // A request defaults to the zero status, so the zero value has to be requestable.
  ondewo::csi::SetControlStatusRequest request;
  EXPECT_EQ(request.control_status(), ondewo::csi::ControlStatus::OK);
}

TEST(TypedApi, ServiceStubsAreConstructibleAgainstAChannel) {
  const std::shared_ptr<grpc::Channel> channel = DeadChannel();
  ASSERT_NE(channel, nullptr);

  std::unique_ptr<ondewo::csi::Conversations::Stub> conversations =
      ondewo::csi::Conversations::NewStub(channel);
  std::unique_ptr<ondewo::s2t::Speech2Text::Stub> s2t =
      ondewo::s2t::Speech2Text::NewStub(channel);
  std::unique_ptr<ondewo::t2s::Text2Speech::Stub> t2s =
      ondewo::t2s::Text2Speech::NewStub(channel);
  std::unique_ptr<ondewo::nlu::Sessions::Stub> sessions =
      ondewo::nlu::Sessions::NewStub(channel);

  EXPECT_NE(conversations, nullptr);
  EXPECT_NE(s2t, nullptr);
  EXPECT_NE(t2s, nullptr);
  EXPECT_NE(sessions, nullptr);
}

TEST(TypedApi, ServicesKeepTheirFullyQualifiedNames) {
  EXPECT_STREQ(ondewo::csi::Conversations::service_full_name(), "ondewo.csi.Conversations");
  EXPECT_STREQ(ondewo::s2t::Speech2Text::service_full_name(), "ondewo.s2t.Speech2Text");
  EXPECT_STREQ(ondewo::t2s::Text2Speech::service_full_name(), "ondewo.t2s.Text2Speech");
  EXPECT_STREQ(ondewo::nlu::Sessions::service_full_name(), "ondewo.nlu.Sessions");
}

// Actually issue an RPC. Nothing is listening, so the only correct outcome is a failure -
// but reaching a transport-level failure means the stub, the request/response types and
// the generated method descriptor all linked and dispatched. A crash or an OK here would
// mean the generated client is broken.
TEST(TypedApi, UnaryRpcAgainstADeadEndpointFailsCleanly) {
  std::unique_ptr<ondewo::csi::Conversations::Stub> conversations =
      ondewo::csi::Conversations::NewStub(DeadChannel());

  grpc::ClientContext client_context;
  client_context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

  ondewo::csi::S2sPipelineId request;
  request.set_id("my-s2s-pipeline");
  ondewo::csi::S2sPipeline response;

  const grpc::Status status =
      conversations->GetS2sPipeline(&client_context, request, &response);

  EXPECT_FALSE(status.ok()) << "an RPC to a dead endpoint reported success";
  EXPECT_TRUE(status.error_code() == grpc::StatusCode::UNAVAILABLE ||
              status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED)
      << "unexpected status " << status.error_code() << ": " << status.error_message();
}

// S2sStream is bidirectional, so it gets its own generated ClientReaderWriter type. Driving
// one proves that half of the generated service compiled and dispatches too.
TEST(TypedApi, BidiStreamingRpcStubIsUsable) {
  std::unique_ptr<ondewo::csi::Conversations::Stub> conversations =
      ondewo::csi::Conversations::NewStub(DeadChannel());

  grpc::ClientContext client_context;
  client_context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

  std::unique_ptr<grpc::ClientReaderWriter<ondewo::csi::S2sStreamRequest,
                                           ondewo::csi::S2sStreamResponse>>
      stream(conversations->S2sStream(&client_context));
  ASSERT_NE(stream, nullptr);

  ondewo::csi::S2sStreamRequest request;
  request.set_pipeline_id("my-s2s-pipeline");
  request.set_session_id("a-session");
  stream->Write(request);
  stream->WritesDone();

  ondewo::csi::S2sStreamResponse response;
  EXPECT_FALSE(stream->Read(&response)) << "a dead endpoint returned a streamed response";

  const grpc::Status status = stream->Finish();
  EXPECT_FALSE(status.ok()) << "a stream to a dead endpoint reported success";
}

// GetControlStream is SERVER-streaming, a different generated shape from the bidi stream
// above: a ClientReader with the request passed by value at construction and no WritesDone.
TEST(TypedApi, ServerStreamingRpcStubIsUsable) {
  std::unique_ptr<ondewo::csi::Conversations::Stub> conversations =
      ondewo::csi::Conversations::NewStub(DeadChannel());

  grpc::ClientContext client_context;
  client_context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

  ondewo::csi::ControlStreamRequest request;
  std::unique_ptr<grpc::ClientReader<ondewo::csi::ControlStreamResponse>> stream(
      conversations->GetControlStream(&client_context, request));
  ASSERT_NE(stream, nullptr);

  ondewo::csi::ControlStreamResponse response;
  EXPECT_FALSE(stream->Read(&response)) << "a dead endpoint returned a streamed response";

  const grpc::Status status = stream->Finish();
  EXPECT_FALSE(status.ok()) << "a stream to a dead endpoint reported success";
}

}  // namespace
}  // namespace ondewo_client_test
