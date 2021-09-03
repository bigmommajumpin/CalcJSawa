#ifndef APPS_PROBABILITY_ABSTRACT_DYNAMIC_DATA_SOURCE_H
#define APPS_PROBABILITY_ABSTRACT_DYNAMIC_DATA_SOURCE_H

#include <escher/table_view_data_source.h>

namespace Probability {

class DynamicTableViewDataSourceDelegate {
public:
  virtual void tableViewDataSourceDidChangeSize() = 0;
};

class DynamicTableViewDataSource {
public:
  DynamicTableViewDataSource(DynamicTableViewDataSourceDelegate * delegate = nullptr) :
      m_dynamicDatSourceDelegate(delegate) {}
  void registerDelegate(DynamicTableViewDataSourceDelegate * delegate) { m_dynamicDatSourceDelegate = delegate; }

protected:
  void notify() {
    if (m_dynamicDatSourceDelegate) {
      m_dynamicDatSourceDelegate->tableViewDataSourceDidChangeSize();
    }
  }

private:
  DynamicTableViewDataSourceDelegate * m_dynamicDatSourceDelegate;
};

}  // namespace Probability

#endif /* APPS_PROBABILITY_ABSTRACT_DYNAMIC_DATA_SOURCE_H */
