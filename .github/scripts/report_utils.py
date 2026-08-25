#!/usr/bin/env python3
"""
TimeEngine CI - Shared Report Utilities
Handles aggregating issues, writing GitHub Step Summaries, creating GitHub annotations,
and generating styled HTML reports for email delivery.
"""

import os
import sys
import html
from typing import List, Dict, Any

class Issue:
    def __init__(self, file_path: str, line_number: int, rule_id: str, message: str, snippet: str = "", severity: str = "error"):
        self.file_path = file_path
        self.line_number = line_number
        self.rule_id = rule_id
        self.message = message
        self.snippet = snippet
        self.severity = severity

class ReportGenerator:
    def __init__(self, title: str, description: str):
        self.title = title
        self.description = description
        self.issues: List[Issue] = []

    def add_issue(self, file_path: str, line_number: int, rule_id: str, message: str, snippet: str = "", severity: str = "error"):
        self.issues.append(Issue(file_path, line_number, rule_id, message, snippet, severity))

    @property
    def has_issues(self) -> bool:
        return len(self.issues) > 0

    @property
    def total_count(self) -> int:
        return len(self.issues)

    def write_github_annotations(self):
        """Emits workflow command annotations for GitHub Actions UI."""
        for issue in self.issues:
            severity = issue.severity.lower()
            if severity not in ("error", "warning", "notice"):
                severity = "error"
            loc_str = ""
            if issue.file_path:
                loc_str = f"file={issue.file_path}"
                if issue.line_number > 0:
                    loc_str += f",line={issue.line_number}"
            prefix = f"::{severity} {loc_str}::" if loc_str else f"::{severity}::"
            escaped_msg = issue.message.replace("\n", "%0A").replace("\r", "%0D")
            print(f"{prefix}[{issue.rule_id}] {escaped_msg}", file=sys.stderr)

    def generate_markdown(self) -> str:
        """Generates Markdown for $GITHUB_STEP_SUMMARY."""
        status_icon = "❌ FAILED" if self.has_issues else "✅ PASSED"
        md = [
            f"# {self.title} - {status_icon}",
            f"\n{self.description}\n",
            f"**Total Issues Found:** `{self.total_count}`\n",
        ]

        if not self.has_issues:
            md.append("> [!TIP]\n> **Clean check!** All commits and PR messaging comply with guidelines.")
            return "\n".join(md)

        md.append("> [!CAUTION]\n> **Action Required**: The following issues were detected. All issues must be addressed before this branch can be merged.\n")
        md.append("| # | Target | Line | Rule | Violation Details | Message / Excerpt |")
        md.append("|---|--------|------|------|-------------------|-------------------|")

        for idx, issue in enumerate(self.issues, 1):
            file_link = f"`{issue.file_path}`" if issue.file_path else "N/A"
            line_str = f"`{issue.line_number}`" if issue.line_number > 0 else "-"
            snippet_str = f"<code>{html.escape(issue.snippet)}</code>" if issue.snippet else "-"
            clean_msg = issue.message.replace("|", "\\|").replace("\n", " ")
            md.append(f"| {idx} | {file_link} | {line_str} | `{issue.rule_id}` | {clean_msg} | {snippet_str} |")

        return "\n".join(md)

    def generate_html(self, causer_email: str = "", trigger_context: str = "") -> str:
        """Generates a responsive HTML email body."""
        status_color = "#e53e3e" if self.has_issues else "#38a169"
        status_title = f"{self.total_count} Issues Detected" if self.has_issues else "All Checks Passed"

        rows = []
        for idx, issue in enumerate(self.issues, 1):
            snippet_html = f'<pre style="margin:4px 0;padding:6px;background:#f7fafc;border:1px solid #e2e8f0;border-radius:4px;font-size:12px;overflow-x:auto;">{html.escape(issue.snippet)}</pre>' if issue.snippet else ''
            rows.append(f"""
            <tr style="border-bottom: 1px solid #e2e8f0;">
                <td style="padding: 10px; font-weight: bold; color: #718096; text-align: center;">{idx}</td>
                <td style="padding: 10px; font-family: monospace; font-size: 13px; color: #2d3748;">{html.escape(issue.file_path or 'N/A')}</td>
                <td style="padding: 10px; font-family: monospace; font-size: 13px; color: #4a5568; text-align: center;">{issue.line_number if issue.line_number > 0 else '-'}</td>
                <td style="padding: 10px;"><span style="display:inline-block;padding:2px 8px;border-radius:12px;background:#fed7d7;color:#9b2c2c;font-size:11px;font-weight:bold;">{html.escape(issue.rule_id)}</span></td>
                <td style="padding: 10px; color: #2d3748; font-size: 13px;">
                    <div>{html.escape(issue.message)}</div>
                    {snippet_html}
                </td>
            </tr>
            """)

        table_html = f"""
        <table style="width: 100%; border-collapse: collapse; margin-top: 15px; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;">
            <thead>
                <tr style="background: #edf2f7; text-align: left; font-size: 12px; color: #4a5568; text-transform: uppercase; letter-spacing: 0.5px;">
                    <th style="padding: 10px; text-align: center;">#</th>
                    <th style="padding: 10px;">Target</th>
                    <th style="padding: 10px; text-align: center;">Line</th>
                    <th style="padding: 10px;">Rule</th>
                    <th style="padding: 10px;">Details & Excerpt</th>
                </tr>
            </thead>
            <tbody>
                {''.join(rows)}
            </tbody>
        </table>
        """ if self.has_issues else "<p style='color:#38a169;font-weight:bold;'>No issues were found. Excellent work!</p>"

        return f"""
        <!DOCTYPE html>
        <html>
        <head>
            <meta charset="utf-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>{html.escape(self.title)} Report</title>
        </head>
        <body style="margin: 0; padding: 20px; background-color: #f7fafc; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;">
            <div style="max-width: 900px; margin: 0 auto; background: #ffffff; border-radius: 8px; overflow: hidden; box-shadow: 0 4px 6px rgba(0, 0, 0, 0.05); border: 1px solid #e2e8f0;">
                <div style="background: #1a202c; padding: 24px; color: #ffffff;">
                    <div style="font-size: 12px; text-transform: uppercase; letter-spacing: 1px; color: #a0aec0; margin-bottom: 4px;">TimeEngine CI Audit Notification</div>
                    <h1 style="margin: 0; font-size: 22px; font-weight: 700;">{html.escape(self.title)}</h1>
                    <div style="margin-top: 8px; font-size: 14px; color: #cbd5e0;">
                        Author: <strong>{html.escape(causer_email or 'Contributor')}</strong> &bull; Trigger: <strong>{html.escape(trigger_context or 'CI Run')}</strong>
                    </div>
                </div>
                
                <div style="padding: 24px;">
                    <div style="border-left: 4px solid {status_color}; padding-left: 12px; margin-bottom: 20px;">
                        <h2 style="margin: 0; font-size: 18px; color: {status_color};">{status_title}</h2>
                        <p style="margin: 4px 0 0 0; color: #4a5568; font-size: 14px;">{html.escape(self.description)}</p>
                    </div>

                    {table_html}

                    <div style="margin-top: 30px; padding: 16px; background: #ebf8ff; border-radius: 6px; border: 1px solid #bee3f8; font-size: 13px; color: #2b6cb0;">
                        <strong>Note:</strong> All commits must follow the repository formatting conventions: <code>&lt;Tag&gt;: &lt;Description&gt;</code> with headers strictly less than 50 characters.
                    </div>
                </div>

                <div style="background: #edf2f7; padding: 12px 24px; font-size: 12px; color: #718096; text-align: center;">
                    TimeEngine Continuous Integration System &bull; Automated Governance
                </div>
            </div>
        </body>
        </html>
        """

    def export_reports(self, output_dir: str = ".github/reports", base_name: str = "report", causer_email: str = "", trigger_context: str = ""):
        os.makedirs(output_dir, exist_ok=True)
        
        step_summary_file = os.environ.get("GITHUB_STEP_SUMMARY")
        if step_summary_file:
            with open(step_summary_file, "a", encoding="utf-8") as f:
                f.write(self.generate_markdown() + "\n\n")

        html_path = os.path.join(output_dir, f"{base_name}.html")
        with open(html_path, "w", encoding="utf-8") as f:
            f.write(self.generate_html(causer_email=causer_email, trigger_context=trigger_context))

        txt_path = os.path.join(output_dir, f"{base_name}.txt")
        with open(txt_path, "w", encoding="utf-8") as f:
            f.write(self.generate_markdown())

        self.write_github_annotations()
