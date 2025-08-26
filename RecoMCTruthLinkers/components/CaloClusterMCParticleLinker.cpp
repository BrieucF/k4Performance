#include "Gaudi/Property.h"

#include "edm4hep/ClusterCollection.h"
#include "edm4hep/ClusterMCParticleLinkCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "podio/UserDataCollection.h"

#include "k4FWCore/Transformer.h"

#include <cmath>
#include <limits>
#include <string>

/**
 * Gaudi functional doing a dummy association between calo clusters and MCParticles based on an angular matching, just
 * here to provide a template code to develop other real applications
 */

struct CaloClusterMCParticleLinker final
    : k4FWCore::MultiTransformer<std::tuple<edm4hep::ClusterMCParticleLinkCollection, podio::UserDataCollection<float>>(
          const edm4hep::MCParticleCollection&, const edm4hep::ClusterCollection&)> {
  CaloClusterMCParticleLinker(const std::string& name, ISvcLocator* svcLoc)
      : MultiTransformer(
            name, svcLoc,
            {KeyValues("InputMCParticles", {"MCParticles"}), KeyValues("InputCaloClusters", {"CaloClusters"})},
            {KeyValues("OutputCaloClusterMCParticleLink", {"CaloClusterMCParticleLink"}),
             KeyValues("OutputErecMinusEgenOverEgen", {"ErecMinusEgenOverEgen"})}) {}

  std::tuple<edm4hep::ClusterMCParticleLinkCollection, podio::UserDataCollection<float>>
  operator()(const edm4hep::MCParticleCollection& MCParticles,
             const edm4hep::ClusterCollection& clusters) const override {
    // create the collection we will return
    edm4hep::ClusterMCParticleLinkCollection clusterMCParticleLinkCollection;
    podio::UserDataCollection<float> ErecMinusEgenOverEgeni_datacoll;
    for (const auto& particle : MCParticles) {
      // skip secondary particles
      if (particle.getGeneratorStatus() == 0)
        continue;
      float smallest_angular_distance = std::numeric_limits<float>::max();
      float delta_e_over_gen_e = 0;
      std::size_t cluster_best_match_index = -1;
      // create already the link here to enable efficiency studies, TOCHECK when no match, setFrom wont be called and
      // the link ?gives an empty pointer for the cluster?
      edm4hep::MutableClusterMCParticleLink clusterMCParticleLink;
      clusterMCParticleLink.setTo(particle);
      for (const auto& cluster : clusters) {
        // derive the cluster/MC particle angular distance with theta = arccos(a.b / |a||b|), assuming the cluster
        // points to the IP for a real implementation, we might want to use the real cluster axis and take the magnetic
        // field into account for the matching
        float mag_cluster_position = std::sqrt(cluster.getPosition()[0] * cluster.getPosition()[0] +
                                               cluster.getPosition()[1] * cluster.getPosition()[1] +
                                               cluster.getPosition()[2] * cluster.getPosition()[2]);
        float mag_particle_momentum = std::sqrt(particle.getMomentum()[0] * particle.getMomentum()[0] +
                                                particle.getMomentum()[1] * particle.getMomentum()[1] +
                                                particle.getMomentum()[2] * particle.getMomentum()[2]);
        float dot = cluster.getPosition()[0] * particle.getMomentum()[0] +
                    cluster.getPosition()[1] * particle.getMomentum()[1] +
                    cluster.getPosition()[2] * particle.getMomentum()[2];
        float cosTheta = dot / (mag_cluster_position * mag_particle_momentum);
        // protection for numerical precision
        if (cosTheta > 1.0)
          cosTheta = 1.0;
        else if (cosTheta < -1.0)
          cosTheta = -1.0;

        // save the information if it is the best match so far
        float current_angular_distance = std::acos(cosTheta);
        if (current_angular_distance < smallest_angular_distance) {
          delta_e_over_gen_e = (cluster.getEnergy() - particle.getEnergy()) / particle.getEnergy();
          smallest_angular_distance = current_angular_distance;
          cluster_best_match_index = cluster.getObjectID().index;
        }
      }
      debug() << "Best match angular distance " << smallest_angular_distance << endmsg;
      // fill the output entries if the matching is valid
      if (smallest_angular_distance < m_cutoff_angle) {
        clusterMCParticleLink.setFrom(clusters.at(cluster_best_match_index));
        ErecMinusEgenOverEgeni_datacoll.push_back(delta_e_over_gen_e);
        clusterMCParticleLinkCollection.push_back(clusterMCParticleLink);
      }
    }
    return std::make_tuple(std::move(clusterMCParticleLinkCollection), std::move(ErecMinusEgenOverEgeni_datacoll));
  }

private:
  /// Configurable property to decide whether to produce validation plots
  Gaudi::Property<bool> m_produceValidationDistributions{this, "ProduceValidationDistributions", true,
                                                         "Decide whether to produce validation distributions or not"};

  /// Criteria on the angle between MCparticle and cluster direction to consider the matching valid
  Gaudi::Property<float> m_cutoff_angle{
      this, "CutoffAngleRadian", 0.2,
      "Cut off on the angular distance between cluster and MC particles to be associated [rad]"};
};

DECLARE_COMPONENT(CaloClusterMCParticleLinker)
