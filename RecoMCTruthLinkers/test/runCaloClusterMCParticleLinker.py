#
# Copyright (c) 2020-2024 Key4hep-Project.
#
# This file is part of Key4hep.
# See https://key4hep.github.io/key4hep-doc/ for further info.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
from Gaudi.Configuration import INFO, DEBUG

import os
# run first a simulation to have the needed input ingredients
os.system("ddsim --enableGun --gun.distribution uniform --gun.energy '5*GeV' --gun.particle e- --numberOfEvents 10 --outputFile ALLEGRO_sim.root --random.enableEventSeed --random.seed 42 --compactFile $K4GEO/FCCee/ALLEGRO/compact/ALLEGRO_o1_v03/ALLEGRO_o1_v03.xml")
os.system("k4run $FCCCONFIG/FullSim/ALLEGRO/ALLEGRO_o1_v03/run_digi_reco.py --IOSvc.Input ALLEGRO_sim.root --IOSvc.Output ALLEGRO_sim_digi_reco.root --doTopoClustering False")

# Loading some files with MCParticles and clusters for testing purposes
from k4FWCore import IOSvc
io_svc = IOSvc("IOSvc")
io_svc.Input = "ALLEGRO_sim_digi_reco.root"
io_svc.Output = "ALLEGRO_sim_digi_reco_perf.root"

from Configurables import CaloClusterMCParticleLinker
CaloClusterMCParticleLinker = CaloClusterMCParticleLinker("CaloClusterMCParticleLinker",
        InputMCParticles = ["MCParticles"],
        InputCaloClusters = ["AugmentedEMBCaloClusters"],
        OutputCaloClusterMCParticleLink = ["EMBCaloClusters_MCParticles_Link"],
        ProduceValidationDistributions = True,
        CutoffAngleRadian = 0.2,
        OutputLevel = DEBUG
        )


# Set auditor service
from Configurables import AuditorSvc, ChronoAuditor
chra = ChronoAuditor()
audsvc = AuditorSvc()
audsvc.Auditors = [chra]
CaloClusterMCParticleLinker.AuditExecute = True

from Configurables import EventDataSvc
from k4FWCore import ApplicationMgr
ApplicationMgr(
    TopAlg=[CaloClusterMCParticleLinker],
    EvtSel='NONE',
    EvtMax=-1,
    ExtSvc=[EventDataSvc("EventDataSvc"), audsvc],
    StopOnSignal=True,
)
