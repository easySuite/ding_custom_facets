(function($) {
  "use strict";

  Drupal.behaviors.ding_custom_facets = {
    attach: function(context, settings) {
      var checkboxes = $('.ding-custom-facets-form', context);

      $.each(checkboxes, function() {
        $('.form-type-checkbox').change(function(e) {
          Drupal.TingSearchOverlay();
          window.location = $(e.target).parent().find('a').attr('href');
        });

        $('.form-type-checkbox a').click(function(e) {
          if ($(this).not('[target="_blank"]').length) {
            Drupal.TingSearchOverlay();
          }
        });

        $( ".primary-content .pane-ding-custom-facets").insertAfter("fieldset#edit-backends--2");
      });
    }
  };
})(jQuery);
