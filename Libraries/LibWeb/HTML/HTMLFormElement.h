/*
 * Copyright (c) 2018-2021, Andreas Kling <andreas@ladybird.org>
 * Copyright (c) 2023, Kenneth Myhra <kennethmyhra@serenityos.org>
 * Copyright (c) 2023, Luke Wilde <lukew@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Time.h>
#include <LibWeb/ARIA/Roles.h>
#include <LibWeb/HTML/HTMLElement.h>
#include <LibWeb/HTML/Navigable.h>

namespace Web::HTML {

// https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#attr-fs-method
#define ENUMERATE_FORM_METHOD_ATTRIBUTES          \
    __ENUMERATE_FORM_METHOD_ATTRIBUTE(get, GET)   \
    __ENUMERATE_FORM_METHOD_ATTRIBUTE(post, POST) \
    __ENUMERATE_FORM_METHOD_ATTRIBUTE(dialog, Dialog)

// https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#attr-fs-enctype
#define ENUMERATE_FORM_METHOD_ENCODING_TYPES                                                   \
    __ENUMERATE_FORM_METHOD_ENCODING_TYPE("application/x-www-form-urlencoded", FormUrlEncoded) \
    __ENUMERATE_FORM_METHOD_ENCODING_TYPE("multipart/form-data", FormData)                     \
    __ENUMERATE_FORM_METHOD_ENCODING_TYPE("text/plain", PlainText)

class HTMLFormElement final : public HTMLElement {
    WEB_PLATFORM_OBJECT(HTMLFormElement, HTMLElement);
    GC_DECLARE_ALLOCATOR(HTMLFormElement);

public:
    virtual ~HTMLFormElement() override;

    String action_from_form_element(GC::Ref<HTMLElement> element) const;

    enum class MethodAttributeState {
#define __ENUMERATE_FORM_METHOD_ATTRIBUTE(_, state) state,
        ENUMERATE_FORM_METHOD_ATTRIBUTES
#undef __ENUMERATE_FORM_METHOD_ATTRIBUTE
    };

    MethodAttributeState method_state_from_form_element(GC::Ref<HTMLElement const> element) const;

    enum class EncodingTypeAttributeState {
#define __ENUMERATE_FORM_METHOD_ENCODING_TYPE(_, state) state,
        ENUMERATE_FORM_METHOD_ENCODING_TYPES
#undef __ENUMERATE_FORM_METHOD_ENCODING_TYPE
    };

    EncodingTypeAttributeState encoding_type_state_from_form_element(GC::Ref<HTMLElement> element) const;

    struct SubmitFormOptions {
        bool from_submit_binding = { false };
        UserNavigationInvolvement user_involvement = { UserNavigationInvolvement::None };
    };
    WebIDL::ExceptionOr<void> submit_form(GC::Ref<HTMLElement> submitter, SubmitFormOptions);
    WebIDL::ExceptionOr<void> implicitly_submit_form();

    void reset_form();

    // NOTE: This is for the JS bindings. Use submit_form instead.
    WebIDL::ExceptionOr<void> submit();

    // NOTE: This is for the JS bindings. Use submit_form instead.
    WebIDL::ExceptionOr<void> request_submit(GC::Ptr<Element> submitter);

    // NOTE: This is for the JS bindings. Use submit_form instead.
    void reset();

    void add_associated_element(Badge<FormAssociatedElement>, HTMLElement&);
    void remove_associated_element(Badge<FormAssociatedElement>, HTMLElement&);

    Vector<GC::Ref<DOM::Element>> get_submittable_elements();

    GC::Ref<HTMLFormControlsCollection> elements() const;
    unsigned length() const;

    struct StaticValidationResult {
        bool result;
        GC::RootVector<GC::Ref<DOM::Element>> unhandled_invalid_controls;
    };

    StaticValidationResult statically_validate_constraints();
    bool interactively_validate_constraints();
    WebIDL::ExceptionOr<bool> check_validity();
    WebIDL::ExceptionOr<bool> report_validity();

    // https://www.w3.org/TR/html-aria/#el-form
    virtual Optional<ARIA::Role> default_role() const override { return ARIA::Role::form; }

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#constructing-entry-list
    bool constructing_entry_list() const { return m_constructing_entry_list; }
    void set_constructing_entry_list(bool value) { m_constructing_entry_list = value; }

    WebIDL::ExceptionOr<void> set_method(String const&);

    GC::Ref<DOM::DOMTokenList> rel_list();

    String action() const;
    WebIDL::ExceptionOr<void> set_action(String const&);

private:
    HTMLFormElement(DOM::Document&, DOM::QualifiedName);

    virtual bool is_html_form_element() const override { return true; }

    virtual void initialize(JS::Realm&) override;
    virtual void visit_edges(Cell::Visitor&) override;

    // ^PlatformObject
    virtual Optional<JS::Value> item_value(size_t index) const override;
    virtual JS::Value named_item_value(FlyString const& name) const override;
    virtual Vector<FlyString> supported_property_names() const override;

    virtual void attribute_changed(FlyString const& name, Optional<String> const& old_value, Optional<String> const& value, Optional<FlyString> const& namespace_) override;

    ErrorOr<String> pick_an_encoding() const;

    ErrorOr<void> mutate_action_url(URL::URL parsed_action, Vector<XHR::FormDataEntry> entry_list, String encoding, GC::Ref<Navigable> target_navigable, Bindings::NavigationHistoryBehavior history_handling, UserNavigationInvolvement user_involvement);
    ErrorOr<void> submit_as_entity_body(URL::URL parsed_action, Vector<XHR::FormDataEntry> entry_list, EncodingTypeAttributeState encoding_type, String encoding, GC::Ref<Navigable> target_navigable, Bindings::NavigationHistoryBehavior history_handling, UserNavigationInvolvement user_involvement);
    void get_action_url(URL::URL parsed_action, Vector<XHR::FormDataEntry> entry_list, GC::Ref<Navigable> target_navigable, Bindings::NavigationHistoryBehavior history_handling, UserNavigationInvolvement user_involvement);
    ErrorOr<void> mail_with_headers(URL::URL parsed_action, Vector<XHR::FormDataEntry> entry_list, String encoding, GC::Ref<Navigable> target_navigable, Bindings::NavigationHistoryBehavior history_handling, UserNavigationInvolvement user_involvement);
    ErrorOr<void> mail_as_body(URL::URL parsed_action, Vector<XHR::FormDataEntry> entry_list, EncodingTypeAttributeState encoding_type, String encoding, GC::Ref<Navigable> target_navigable, Bindings::NavigationHistoryBehavior history_handling, UserNavigationInvolvement user_involvement);
    void plan_to_navigate_to(URL::URL url, Variant<Empty, String, POSTResource> post_resource, Vector<XHR::FormDataEntry> entry_list, GC::Ref<Navigable> target_navigable, Bindings::NavigationHistoryBehavior history_handling, UserNavigationInvolvement user_involvement);

    FormAssociatedElement* default_button();
    size_t number_of_fields_blocking_implicit_submission() const;

    bool m_firing_submission_events { false };

    // https://html.spec.whatwg.org/multipage/forms.html#locked-for-reset
    bool m_locked_for_reset { false };

    Vector<GC::Ref<HTMLElement>> m_associated_elements;

    // https://html.spec.whatwg.org/multipage/forms.html#past-names-map
    struct PastNameEntry {
        GC::Ptr<DOM::Node const> node;
        MonotonicTime insertion_time;
    };
    HashMap<FlyString, PastNameEntry> mutable m_past_names_map;

    GC::Ptr<HTMLFormControlsCollection> mutable m_elements;

    bool m_constructing_entry_list { false };

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#planned-navigation
    // Each form element has a planned navigation, which is either null or a task; when the form is first created,
    // its planned navigation must be set to null.
    GC::Ptr<Task const> m_planned_navigation;

    GC::Ptr<DOM::DOMTokenList> m_rel_list;
};

}

namespace Web::DOM {
template<>
inline bool Node::fast_is<HTML::HTMLFormElement>() const { return is_html_form_element(); }
}
