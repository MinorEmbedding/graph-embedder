#ifndef __MAJORMINER_EMBEDDING_VISUALIZER_HPP_
#define __MAJORMINER_EMBEDDING_VISUALIZER_HPP_

#include "majorminer_types.hpp"

#include <sstream>
#include <fstream>

namespace majorminer
{
  typedef std::pair<double, double> Coordinate_t;
  typedef UnorderedMap<fuint32_t, Coordinate_t> VisualizerNodeCoordinateMap;

  class EmbeddingVisualizer
  {
    public:
      EmbeddingVisualizer(const graph_t& source, const graph_t& target, std::string filename)
       : m_source(source), m_target(target), m_filename(filename) { }
      virtual ~EmbeddingVisualizer() {}

      void draw(const embedding_mapping_t& embedding);

    protected:
      virtual fuint32_t insertEdge(Vector<Coordinate_t>& coords, const edge_t& edge) = 0;
      virtual Coordinate_t insertNode(fuint32_t v) const = 0;
      virtual double getWidth() const = 0;
      virtual double getHeight() const = 0;

      double getNodeSize() const { return 50; }
      double getRadius() const { return getNodeSize() / 2; }
      double getStrokeWidth() const { return 6; }

    private:
      void initialize();
      void writeToFile();
      void drawNode(fuint32_t node, double radius, const Coordinate_t& coordinate, const std::string& color, fuint32_t n = 0);
      void drawEdge(const edge_t& edge, const std::string& color, double stroke = 1);
      void drawChains();
      void drawInterChainConnections();
      void drawNodes();
      const std::string& getColor(fuint32_t node);

    private:
      bool m_initialized = false;
      const graph_t& m_source;
      const graph_t& m_target;
      const embedding_mapping_t* m_embedding; // temporary pointer to the embedding
      std::string m_filename;
      fuint32_t m_iteration;
      std::stringstream m_svg;
      std::stringstream m_colorFactory;

      UnorderedMap<edge_t, fuint32_pair_t, PairHashFunc<fuint32_t>> m_edgePtrs;
      Vector<Coordinate_t> m_edgeSamples;
      UnorderedMap<fuint32_t, std::string> m_sourceNodeColors;

    protected:
      VisualizerNodeCoordinateMap m_nodes;
  };

  class ChimeraVisualizer : public EmbeddingVisualizer
  {
    public:
      ChimeraVisualizer(const graph_t& source, const graph_t& target, std::string filename, fuint32_t nbRows, fuint32_t nbCols)
        : EmbeddingVisualizer(source, target, std::move(filename)),
          m_nbRows(nbRows), m_nbCols(nbCols),
          m_nbVerticesPerRow(8 * m_nbCols) {}
      ~ChimeraVisualizer(){}

    protected:
      fuint32_t insertEdge(Vector<Coordinate_t>& coords, const edge_t& edge) override;
      Coordinate_t insertNode(fuint32_t v) const override;
      double getWidth() const override;
      double getHeight() const override;

    private:
      fuint32_t m_nbRows;
      fuint32_t m_nbCols;
      fuint32_t m_nbVerticesPerRow;
  };

  class KingsVisualizer : public EmbeddingVisualizer
  {
    public:
      KingsVisualizer(const graph_t& source, const graph_t& target, std::string filename, fuint32_t nbRows, fuint32_t nbCols)
        : EmbeddingVisualizer(source, target, std::move(filename)),
          m_nbRows(nbRows), m_nbCols(nbCols) {}
      ~KingsVisualizer(){}

    protected:
      fuint32_t insertEdge(Vector<Coordinate_t>& coords, const edge_t& edge) override;
      Coordinate_t insertNode(fuint32_t v) const override;
      double getWidth() const override;
      double getHeight() const override;

    private:
      fuint32_t m_nbRows;
      fuint32_t m_nbCols;
  };
}


#endif
