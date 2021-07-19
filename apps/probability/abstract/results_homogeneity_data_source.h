#ifndef APPS_PROBABILITY_ABSTRACT_RESULTS_HOMOGENEITY_DATA_SOURCE_H
#define APPS_PROBABILITY_ABSTRACT_RESULTS_HOMOGENEITY_DATA_SOURCE_H

#include <escher/table_view_data_source.h>

#include "homogeneity_data_source.h"

namespace Probability {

/* This DataSource reads the statistic and displays the expected values and the totals for each cell
 * position. */
class ResultsHomogeneityDataSource : public Escher::TableViewDataSource {
public:
  ResultsHomogeneityDataSource(HomogeneityStatistic * statistic);
  int numberOfRows() const override { return m_statistic->numberOfResultRows() + 1; }
  int numberOfColumns() const override { return m_statistic->numberOfResultColumns() + 1; }
  KDCoordinate columnWidth(int i) override { return HomogeneityTableDataSource::k_columnWidth; }
  KDCoordinate rowHeight(int j) override { return HomogeneityTableDataSource::k_rowHeight; }
  int typeAtLocation(int i, int j) override { return 0; }
  HighlightCell * reusableCell(int i, int type) override;
  int reusableCellCount(int type) override { return numberOfRows() * numberOfColumns(); };
  void willDisplayCellAtLocation(Escher::HighlightCell * cell, int i, int j) override;

private:
  HomogeneityStatistic * m_statistic;
  EvenOddBufferTextCell m_cells[HomogeneityTableDataSource::k_numberOfReusableCells];
};

/* This class adds a row / column header to the ResultsHomogeneityDataSource. */
class HomogeneityTableDataSourceWithTotals : public HomogeneityTableDataSource {
public:
  HomogeneityTableDataSourceWithTotals(TableViewDataSource * contentTable,
                                       I18n::Message headerPrefix = I18n::Message::Group,
                                       I18n::Message totalMessage = I18n::Message::Total);
  void willDisplayCellAtLocation(Escher::HighlightCell * cell, int column, int row) override;

private:
  I18n::Message m_totalMessage;
};

}  // namespace Probability

#endif /* APPS_PROBABILITY_ABSTRACT_RESULTS_HOMOGENEITY_DATA_SOURCE_H */
