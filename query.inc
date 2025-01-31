<?php
/*
 * Primary function for bulding and displaying a query
 * 
 * @param int $query_id Id for the query
 * @param array $options_override an array for changing or adding query data options
 * @param bool $reset_post_data Reset the $wp_query after execution
 * @return string Can return a string of html based on parameter $return
 */ 
function qw_execute_query($query_id, $options_override = array(), $reset_post_data = true)
{
  global $wp_query;
  // backup the old query
  $old_query = $wp_query;
  $wp_query = null;
  
  // get the query options
  $options = qw_generate_query_options($query_id, $options_override);
  
  // get formatted query arguments
  $args = qw_generate_query_args($options);
  
  // set the new query
  $wp_query = new WP_Query($args);
  
  // get the themed content
  $themed = qw_template_query($wp_query, $options);
  
  // Reset Post Data
  if($reset_post_data){
    wp_reset_postdata();
    $wp_query = $old_query;
  }
  
  return $themed;
}
/*
 * Get the Query, and set $options to defaults
 *
 * @param int $query_id The unique ID of the query
 * @param array $options_override An array of values to override in the retrieved set
 * @return array Of Query options
 */
function qw_generate_query_options($query_id, $options_override = array())
{
  global $wpdb, $wp_query;
  $table_name = $wpdb->prefix."query_wrangler";
  $sql = "SELECT id,name,type,slug,data FROM ".$table_name." WHERE id = ".$query_id;
  
  $rows = $wpdb->get_results($sql);
  
  // unserealize the stored data
  $options = unserialize($rows[0]->data);
  
  // override options
  $options =  array_merge_recursive((array)$options, $options_override);
  
  // build query_details
  $options['meta'] = array();
  $options['meta']['id'] = $rows[0]->id;
  $options['meta']['slug'] = $rows[0]->slug;
  $options['meta']['name'] = $rows[0]->name;
  $options['meta']['type'] = $rows[0]->type;
  $options['meta']['pagination'] = (isset($options['args']['paged'])) ? true : false;
  $options['meta']['header'] = $options['display']['header'];
  $options['meta']['footer'] = $options['display']['footer'];
  $options['meta']['empty'] = $options['display']['empty'];
  
  return $options;
}

/*
 * Generate the WP query itself
 * 
 * @param array $options Query data
 * $param array $options_override can change the options array
 * @return array Query Arguments
 */
function qw_generate_query_args($options = array(), $options_override = array())
{  
  //print_r($options);exit();
  $display = $options['display'];

  // standard arguments
  $args['posts_per_page'] = ($options['args']['posts_per_page']) ? $options['args']['posts_per_page']: 5;
  $args['offset']         = ($options['args']['offset']) ? $options['args']['offset']: 0;
  $args['post_status']    = ($options['args']['post_status']) ? $options['args']['post_status']: 'publish';
  $args['orderby']        = ($options['args']['orderby']) ? $options['args']['orderby']: 'none';
  $args['order']          = ($options['args']['order']) ? $options['args']['order']: 'ASC';
  $args['paged']          = $options['args']['paged'];
  //$args['']              = $options['args'][''];
  
  // page parent
  if($options['args']['post_parent'] != ""){
    $args['post_parent'] = $options['args']['post_parent'];
  }
  
  // post types
  if(is_array($options['args']['post_types'])){
    $args['post_type'] = array_values($options['args']['post_types']);
  }
  
  // category arguments
  if(is_array($options['args']['cat'])){
    // handle normal cat operator as a string
    if($options['args']['cat_operator'] == "cat"){
      $args['cat'] = implode(",", array_keys($options['args']['cat']));      
    }
    // other cat operators are arrays
    else {
      $args[$options['args']['cat_operator']] = array_values($options['args']['cat']);      
    }
  }
  
  // tag arguments
  if(is_array($options['args']['tag'])){
    $args[$options['args']['tag_operator']] = array_keys($options['args']['tag']);      
  }
  
  return $args;
}

/*
 * Get all queries of the type widget
 * 
 * @return array of query widgets with key as query id
 */
function qw_get_all_widgets()
{
  global $wpdb;
  $table_name = $wpdb->prefix."query_wrangler";
  $sql = "SELECT id,name FROM ".$table_name." WHERE type = 'widget'";
  $rows = $wpdb->get_results($sql);
  
  $widgets = array();
  foreach($rows as $row){
    $widgets[$row->id] = $row->name;
  }
  
  return $widgets;
}
/*
 * Get all query pages
 *
 * @return array Query pages in WP post format
 */
function qw_get_all_pages()
{
  global $wpdb;
  $table_name = $wpdb->prefix."query_wrangler";
  $sql = "SELECT id,name,path FROM ".$table_name." WHERE type = 'page'";
  $rows = $wpdb->get_results($sql);
  
  $pages = array();
  $blog_url = get_bloginfo('wpurl');
  
  $i=0;
  foreach($rows as $row){
    $pages[$i] = new stdClass();
    $pages[$i]->ID = $row->id;
    $pages[$i]->title = $row-name;
    $pages[$i]->post_title = $row->name;
    $pages[$i]->guid = $blog_url.$row->path;
    $pages[$i]->post_type = 'page';
  }
  
  return $pages;  
}

/*
 * Default values for  new query
 * 
 * @return array Default query settings
 */
function qw_default_query_data(){
  return array(
    'args' => array(
      'posts_per_page'  => 5,
      'offset'          => 0,
      'cat_operator'    => 'cat',
      'post_status'     => 'published',
      'orderby'         => 'date',
      'order'           => 'ASC',
    ),
    'display' => array(
      'type' => 'full',
      'full_settings' => array(
        'style' => 'complete',
      ),
      'field_settings' => array(
        'style' => 'unformatted',
      ),
    ),
  );
}
/*
 * Slug creation
 */
function qw_make_slug($string){
  return stripcslashes(preg_replace('/[\s_\'\"]/','_', strtolower(strip_tags($string))));
}
/*
 * usort callback. I likely stole this from somewhere.. like php.net
 */
function qw_cmp($a,$b) {
  if ($a['weight'] == $b['weight']) return 0;
  return ($a['weight'] < $b['weight'])? -1 : 1;
}
