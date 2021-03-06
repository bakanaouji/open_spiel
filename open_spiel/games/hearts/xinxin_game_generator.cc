// Copyright 2019 DeepMind Technologies Ltd. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Copyright 2019 DeepMind Technologies Ltd. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <fstream>
#include <iostream>
#include <limits>

#include "open_spiel/abseil-cpp/absl/flags/flag.h"
#include "open_spiel/abseil-cpp/absl/flags/parse.h"
#include "open_spiel/abseil-cpp/absl/random/distributions.h"
#include "open_spiel/abseil-cpp/absl/time/clock.h"
#include "open_spiel/abseil-cpp/absl/time/time.h"
#include "open_spiel/algorithms/evaluate_bots.h"
#include "open_spiel/games/hearts/hearts/Hearts.h"
#include "open_spiel/games/hearts/xinxin_bot.h"
#include "open_spiel/spiel.h"
#include "open_spiel/spiel_bots.h"

ABSL_FLAG(int, num_games, 5, "How many games to run.");
ABSL_FLAG(std::string, path, "/tmp/xinxin_logs.txt",
          "Where to output the logs.");

namespace open_spiel {
namespace {

uint_fast32_t Seed() { return absl::ToUnixMicros(absl::Now()); }

void XinxinBot_GenerateGames(int num_games, std::string path) {
  std::mt19937 rng(Seed());
  std::ofstream game_logs(path);
  auto game = open_spiel::LoadGame("hearts");
  std::vector<std::unique_ptr<open_spiel::Bot>> bots;
  std::vector<Bot *> bot_ptrs;

  for (int i = 0; i < hearts::kNumPlayers; i++) {
    bots.push_back(open_spiel::hearts::MakeXinxinBot(game->GetParameters(),
                                                     game->NumPlayers()));
    bot_ptrs.push_back(bots.back().get());
  }

  for (int i = 0; i < num_games; i++) {
    std::unique_ptr<open_spiel::State> state = game->NewInitialState();
    const int num_players = bots.size();
    for (auto bot : bot_ptrs) bot->Restart();
    while (!state->IsTerminal()) {
      if (state->IsChanceNode()) {
        Action action = SampleAction(state->ChanceOutcomes(), rng).first;
        for (auto bot : bot_ptrs)
          bot->InformAction(*state, kChancePlayerId, action);
        state->ApplyAction(action);
      } else {
        Player current_player = state->CurrentPlayer();
        Action action = bots[current_player]->Step(*state);
        for (Player p = 0; p < num_players; ++p) {
          if (p != current_player) {
            bots[p]->InformAction(*state, current_player, action);
          }
        }
        state->ApplyAction(action);
      }
    }
    for (Player p = 0; p < num_players; ++p) {
      // allows checking for differences in the returns
      bots[p]->InformAction(*state, kTerminalPlayerId, kInvalidAction);
    }
    game_logs << absl::StrJoin(state->History(), " ") << "\n";
  }
  game_logs.close();
}

}  // namespace
}  // namespace open_spiel

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);
  open_spiel::XinxinBot_GenerateGames(absl::GetFlag(FLAGS_num_games),
                                      absl::GetFlag(FLAGS_path));
}
