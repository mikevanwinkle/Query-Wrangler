<?php
/*
 * $style - field style
 * $rows - a processed array of rows fields and classes
 * $query_details - other query details
 */
?>
<table class="qw-query-table query-<?php print $slug; ?>" cellpadding="0" cellspacing="0" border="0">
  <thead class="qw-query-table-head">
    <?php foreach($rows[0]['fields'] as $field): ?>
      <th><?php print $field['label']; ?></th>
    <?php endforeach; ?>
  </thead>
  <tbody class="qw-query-table-body">
    <?php foreach($rows as $row): ?>
      <tr class="<?php print $row['row_classes']; ?>">
        
        <?php foreach($row['fields'] as $field): ?>
          <?php if(isset($field['output'])): ?>
            <td class="<?php print $field['classes']; ?>">
              <?php print $field['output']; ?>
            </td>
          <?php endif; ?>
        <?php endforeach; ?>
      
      </tr>
    <?php endforeach; ?>
  </tbody>
</table>