/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "PriceUIElement.h"
#include "Item.h"
#include "ItemsCache.h"

void PriceUIElement::RegisterObject(Context* context)
{
    context->RegisterFactory<PriceUIElement>();
}

PriceUIElement::PriceUIElement(Context* context) :
    UIElement(context)
{
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    SetLayoutMode(LM_HORIZONTAL);
}

PriceUIElement::~PriceUIElement() = default;

void PriceUIElement::Add(uint32_t index, uint32_t count)
{
    auto* itemsCache = GetSubsystem<ItemsCache>();
    Item* item = itemsCache->Get(index);
    if (!item)
        return;

    auto* container = CreateChild<UIElement>();
    container->SetLayoutMode(LM_HORIZONTAL);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    auto* icon = container->CreateChild<BorderImage>();
    icon->SetSize({ 16, 16 });
    icon->SetMaxSize({ 16, 16 });
    icon->SetMinSize({ 16, 16 });

    Texture2D* texture = (index != 9999999) ? cache->GetResource<Texture2D>(item->iconFile_) :
        cache->GetResource<Texture2D>("Textures/Icons/DrachmaCoin.png");
    icon->SetTexture(texture);
    icon->SetFullImageRect();

    auto* countText = container->CreateChild<Text>();
    countText->SetText(String(count));
    countText->SetStyleAuto();
    countText->SetFontSize(8);

    container->SetMinWidth(countText->GetWidth() + 24);
}
