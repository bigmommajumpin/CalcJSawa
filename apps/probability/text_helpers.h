#ifndef APPS_PROBABILITY_TEXT_HELPERS_H
#define APPS_PROBABILITY_TEXT_HELPERS_H

#include <apps/i18n.h>
#include <apps/shared/text_field_delegate_app.h>
#include <assert.h>
#include <escher/container.h>

#include "models/data.h"

namespace Probability {

/* Fake sprintf, only supports %s or %c */
int sprintf(char * buffer, const char * format, ...);

int testToText(Data::Test t, char * buffer, int bufferLength);
const char * testToTextSymbol(Data::Test t);
int testTypeToText(Data::TestType t, char * buffer, int bufferLength);


}  // namespace Probability

#endif /* APPS_PROBABILITY_TEXT_HELPERS_H */
