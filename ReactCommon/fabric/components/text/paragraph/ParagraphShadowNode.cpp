/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ParagraphShadowNode.h"
#include "ParagraphLocalData.h"
#include "ParagraphMeasurementCache.h"

namespace facebook {
namespace react {

const char ParagraphComponentName[] = "Paragraph";

AttributedString ParagraphShadowNode::getAttributedString() const {
  if (!cachedAttributedString_.has_value()) {
    auto textAttributes = TextAttributes::defaultTextAttributes();
    textAttributes.apply(getProps()->textAttributes);

    cachedAttributedString_ = BaseTextShadowNode::getAttributedString(
        textAttributes, shared_from_this());
  }

  return cachedAttributedString_.value();
}

void ParagraphShadowNode::setTextLayoutManager(
    SharedTextLayoutManager textLayoutManager) {
  ensureUnsealed();
  textLayoutManager_ = textLayoutManager;
}

void ParagraphShadowNode::setMeasureCache(
    SharedParagraphMeasurementCache cache) {
  ensureUnsealed();
  measureCache_ = cache;
}

void ParagraphShadowNode::updateLocalDataIfNeeded() {
  ensureUnsealed();

  auto attributedString = getAttributedString();
  auto currentLocalData =
      std::static_pointer_cast<const ParagraphLocalData>(getLocalData());
  if (currentLocalData &&
      currentLocalData->getAttributedString() == attributedString) {
    return;
  }

  auto localData = std::make_shared<ParagraphLocalData>();
  localData->setAttributedString(std::move(attributedString));
  localData->setTextLayoutManager(textLayoutManager_);
  setLocalData(localData);
}

#pragma mark - LayoutableShadowNode

Size ParagraphShadowNode::measure(LayoutConstraints layoutConstraints) const {
  AttributedString attributedString = getAttributedString();
  const ParagraphAttributes attributes = getProps()->paragraphAttributes;

  // Cache results of this function so we don't need to call measure()
  // repeatedly
  ParagraphMeasurementCacheKey hashValue =
      std::make_tuple(attributedString, attributes, layoutConstraints);
  if (measureCache_->exists(hashValue)) {
    return measureCache_->get(hashValue);
  }

  Size measuredSize = textLayoutManager_->measure(
      attributedString, getProps()->paragraphAttributes, layoutConstraints);

  measureCache_->set(hashValue, measuredSize);

  return measuredSize;
}

void ParagraphShadowNode::layout(LayoutContext layoutContext) {
  updateLocalDataIfNeeded();
  ConcreteViewShadowNode::layout(layoutContext);
}

} // namespace react
} // namespace facebook
