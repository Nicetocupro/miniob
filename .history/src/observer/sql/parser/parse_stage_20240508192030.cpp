/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Longda on 2021/4/13.
//

#include <string.h>
#include <string>

#include "parse_stage.h"

#include "common/conf/ini.h"
#include "common/io/io.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "event/session_event.h"
#include "event/sql_event.h"
#include "sql/parser/parse.h"

using namespace common;

RC ParseStage::handle_request(SQLStageEvent *sql_event)
{
  RC rc = RC::SUCCESS;
  
  SqlResult *sql_result = sql_event->session_event()->sql_result();
  const std::string &sql = sql_event->sql();

  ParsedSqlResult parsed_sql_result;

  parse(sql.c_str(), &parsed_sql_result);

/*
  // 遍历 sql_nodes_ 向量，找到 SelectSqlNode 对象并提取它
  SelectSqlNode selection_node_ptr;
  for (auto it = parsed_sql_result.sql_nodes().begin(); it != parsed_sql_result.sql_nodes().end(); ++it) {
    if ((*it)->flag == SqlCommandFlag::SCF_SELECT) {
        // 移动 SelectSqlNode 对象到 selection_node_ptr
        selection_node_ptr = std::move((*it)->selection);
        break; // 找到一个就可以退出循环
      }
  }

  for(int i = 0 ; i < selection_node_ptr.conditions.size() ;i++)
  {
    LOG_WARN("condition right_value:type=%d , data=%d, left_value:type=%d ,data=%d",
    selection_node_ptr.conditions[i].right_value.attr_type() ,selection_node_ptr.conditions[i].right_value.get_int(),
    selection_node_ptr.conditions[i].left_value.attr_type() ,selection_node_ptr.conditions[i].left_value.get_int()
    );
  }
*/

  if (parsed_sql_result.sql_nodes().empty()) {
    sql_result->set_return_code(RC::SUCCESS);
    sql_result->set_state_string("");
    return RC::INTERNAL;
  }

  if (parsed_sql_result.sql_nodes().size() > 1) {
    LOG_WARN("got multi sql commands but only 1 will be handled");
  }

   std::unique_ptr<ParsedSqlNode> sql_node = std::move(parsed_sql_result.sql_nodes().front());
  if (sql_node == SCF_ERROR) {
    sql_debug(sql_node->error.error_msg.c_str());
    // set error information to event
    rc = RC::SQL_SYNTAX;
    sql_result->set_return_code(rc);
    if(sql_node->error.flag)
    {
      sql_result->set_state_string("");
    }
    else
    {
      // sql_result->set_state_string("Failed to parse sql");
      sql_result->set_state_string("");
    } 
    
    return rc;
  }

  sql_event->set_sql_node(std::move(sql_node));

  return RC::SUCCESS;
}