#include "graph_view.h"
#include <float.h>

using namespace Shared;

namespace Graph {

GraphView::GraphView(StorageCartesianFunctionStore * functionStore, InteractiveCurveViewRange * graphRange,
  CurveViewCursor * cursor, BannerView * bannerView, View * cursorView) :
  FunctionGraphView(graphRange, cursor, bannerView, cursorView),
  m_functionStore(functionStore),
  m_tangent(false)
{
}

void GraphView::reload() {
  if (m_tangent) {
    KDRect dirtyZone(KDRect(0, 0, bounds().width(), bounds().height()-m_bannerView->bounds().height()));
    markRectAsDirty(dirtyZone);
  }
  return FunctionGraphView::reload();
}

void GraphView::drawRect(KDContext * ctx, KDRect rect) const {
  FunctionGraphView::drawRect(ctx, rect);
  for (int i = 0; i < m_functionStore->numberOfActiveFunctions(); i++) {
    Ion::Storage::Record record = m_functionStore->activeRecordAtIndex(i);
    ExpiringPointer<CartesianFunction> f = m_functionStore->modelForRecord(record);;

    /* Draw function (color the area under curve of the selected function) */
    if (record == m_selectedRecord) {
      drawCurve(ctx, rect, [](float t, void * model, void * context) {
        CartesianFunction * f = (CartesianFunction *)model;
        Poincare::Context * c = (Poincare::Context *)context;
        return f->evaluateAtAbscissa(t, c);
      }, f.operator->(), context(), f->color(), true, m_highlightedStart, m_highlightedEnd);
    } else {
      drawCurve(ctx, rect, [](float t, void * model, void * context) {
        CartesianFunction * f = (CartesianFunction *)model;
        Poincare::Context * c = (Poincare::Context *)context;
        return f->evaluateAtAbscissa(t, c);
      }, f.operator->(), context(), f->color());
    }

    /* Draw tangent */
    if (m_tangent && record == m_selectedRecord) {
      float tangentParameter[2];
      tangentParameter[0] = f->approximateDerivative(m_curveViewCursor->x(), context());
      tangentParameter[1] = -tangentParameter[0]*m_curveViewCursor->x()+f->evaluateAtAbscissa(m_curveViewCursor->x(), context());
      drawCurve(ctx, rect, [](float t, void * model, void * context) {
          float * tangent = (float *)model;
          return tangent[0]*t+tangent[1];
        }, tangentParameter, nullptr, Palette::GreyVeryDark);
    }
  }
}

}
