#!/usr/bin/env python3
"""
TimeEngine CI - Professional Email Template & Dispatcher
Generates beautiful, responsive, brand-styled HTML email alerts for CI build & guideline events.
"""

import os
import sys
import html
import argparse

def generate_email_html(
    workflow_name: str,
    status: str,
    repo_name: str,
    branch_name: str,
    commit_sha: str,
    author_name: str,
    author_email: str,
    run_url: str,
    error_summary: str = ""
) -> str:
    is_failed = status.upper() in ("FAILED", "FAILURE")
    accent_color = "#e53e3e" if is_failed else "#38a169"
    badge_bg = "#fed7d7" if is_failed else "#c6f6d5"
    badge_text = "#9b2c2c" if is_failed else "#22543d"
    status_label = "BUILD FAILED" if is_failed else "BUILD SUCCEEDED"
    short_sha = commit_sha[:8] if commit_sha else "HEAD"

    error_section = ""
    if error_summary and error_summary.strip():
        error_section = f"""
        <div style="margin-top: 24px;">
            <div style="font-size: 13px; font-weight: 700; text-transform: uppercase; letter-spacing: 0.5px; color: #4a5568; margin-bottom: 8px;">
                Diagnostic Summary & Details
            </div>
            <div style="background: #fff5f5; border: 1px solid #feb2b2; border-radius: 6px; padding: 14px; font-family: 'SFMono-Regular', Consolas, 'Liberation Mono', Menlo, monospace; font-size: 13px; color: #742a2a; line-height: 1.5; white-space: pre-wrap; overflow-x: auto;">
{html.escape(error_summary)}
            </div>
        </div>
        """

    return f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>TimeEngine CI: {html.escape(workflow_name)} {status_label}</title>
</head>
<body style="margin: 0; padding: 30px 15px; background-color: #f4f6f8; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; color: #2d3748; -webkit-font-smoothing: antialiased;">
    <table align="center" border="0" cellpadding="0" cellspacing="0" width="100%" style="max-width: 680px; margin: 0 auto; background-color: #ffffff; border-radius: 10px; overflow: hidden; box-shadow: 0 10px 25px rgba(0, 0, 0, 0.06); border: 1px solid #e2e8f0;">
        <!-- Header -->
        <tr>
            <td style="background: linear-gradient(135deg, #111827 0%, #1f2937 100%); padding: 30px 32px; border-bottom: 3px solid {accent_color};">
                <table width="100%" border="0" cellpadding="0" cellspacing="0">
                    <tr>
                        <td>
                            <div style="display: inline-block; font-size: 11px; font-weight: 800; text-transform: uppercase; letter-spacing: 1.5px; color: #9ca3af; margin-bottom: 6px;">
                                ⏱️ TIMEENGINE CONTINUOUS INTEGRATION
                            </div>
                            <h1 style="margin: 0; font-size: 22px; font-weight: 700; color: #ffffff; letter-spacing: -0.3px;">
                                {html.escape(workflow_name)}
                            </h1>
                        </td>
                        <td align="right" valign="middle">
                            <span style="display: inline-block; padding: 6px 14px; border-radius: 20px; font-size: 12px; font-weight: 700; text-transform: uppercase; letter-spacing: 0.5px; background-color: {badge_bg}; color: {badge_text};">
                                {status_label}
                            </span>
                        </td>
                    </tr>
                </table>
            </td>
        </tr>

        <!-- Body Content -->
        <tr>
            <td style="padding: 32px;">
                <div style="font-size: 15px; line-height: 1.6; color: #374151; margin-bottom: 24px;">
                    Hello <strong>{html.escape(author_name or 'Contributor')}</strong>,<br>
                    The automated CI pipeline for <strong>{html.escape(repo_name)}</strong> encountered an issue while validating your recent changes.
                </div>

                <!-- Metadata Card -->
                <div style="background-color: #f8fafc; border: 1px solid #e2e8f0; border-radius: 8px; padding: 18px 20px;">
                    <table width="100%" border="0" cellpadding="0" cellspacing="0" style="font-size: 13px; line-height: 1.8;">
                        <tr>
                            <td width="30%" style="color: #64748b; font-weight: 600;">Repository:</td>
                            <td width="70%" style="color: #1e293b; font-weight: 500;">{html.escape(repo_name)}</td>
                        </tr>
                        <tr>
                            <td style="color: #64748b; font-weight: 600;">Branch / Target:</td>
                            <td style="color: #1e293b; font-weight: 600; font-family: monospace;">{html.escape(branch_name)}</td>
                        </tr>
                        <tr>
                            <td style="color: #64748b; font-weight: 600;">Commit SHA:</td>
                            <td style="color: #1e293b; font-family: monospace;">
                                <a href="https://github.com/{html.escape(repo_name)}/commit/{html.escape(commit_sha)}" style="color: #2563eb; text-decoration: none;">{short_sha}</a>
                            </td>
                        </tr>
                        <tr>
                            <td style="color: #64748b; font-weight: 600;">Triggered By:</td>
                            <td style="color: #1e293b;">{html.escape(author_name)} ({html.escape(author_email or 'N/A')})</td>
                        </tr>
                    </table>
                </div>

                {error_section}

                <!-- Action Button -->
                <div style="margin-top: 32px; text-align: center;">
                    <a href="{html.escape(run_url)}" style="display: inline-block; padding: 12px 28px; background: linear-gradient(180deg, #1f2937 0%, #111827 100%); color: #ffffff; text-decoration: none; font-size: 14px; font-weight: 600; border-radius: 6px; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);">
                        Inspect Build Logs & Artifacts &rarr;
                    </a>
                </div>

                <div style="margin-top: 28px; padding-top: 20px; border-top: 1px solid #edf2f7; font-size: 12px; color: #64748b; line-height: 1.5;">
                    💡 <em>Tip: You can run checks locally before pushing using our project scripts in <code>Scripts/Windows/</code> or <code>Scripts/Linux/</code>, and Premake action <code>premake5 check-rules</code>.</em>
                </div>
            </td>
        </tr>

        <!-- Footer -->
        <tr>
            <td style="background-color: #f8fafc; padding: 18px 32px; border-top: 1px solid #e2e8f0; text-align: center; font-size: 12px; color: #94a3b8;">
                TimeEngine Automated Build Governance &bull; High-Performance 2D/3D Engine
            </td>
        </tr>
    </table>
</body>
</html>"""

def main():
    parser = argparse.ArgumentParser(description="TimeEngine CI Professional Email Dispatcher")
    parser.add_argument("--workflow", required=True, help="Workflow Name")
    parser.add_argument("--status", default="FAILED", help="Build Status (FAILED / PASSED)")
    parser.add_argument("--repo", default="1SHAMAY1/TimeEngine", help="Repository Name")
    parser.add_argument("--branch", default="main", help="Branch / Ref name")
    parser.add_argument("--commit", default="", help="Commit SHA")
    parser.add_argument("--author", default="", help="Author Login / Name")
    parser.add_argument("--email", default="", help="Author Email Address")
    parser.add_argument("--run-url", default="", help="GitHub Action Run URL")
    parser.add_argument("--summary-file", default="", help="Path to text summary file")
    parser.add_argument("--output", default=".github/reports/notification.html", help="Path to write HTML file")
    args = parser.parse_args()

    error_summary = ""
    if args.summary_file and os.path.exists(args.summary_file):
        try:
            with open(args.summary_file, "r", encoding="utf-8", errors="replace") as f:
                error_summary = f.read()
        except Exception:
            pass

    html_content = generate_email_html(
        workflow_name=args.workflow,
        status=args.status,
        repo_name=args.repo,
        branch_name=args.branch,
        commit_sha=args.commit,
        author_name=args.author,
        author_email=args.email,
        run_url=args.run_url,
        error_summary=error_summary
    )

    out_dir = os.path.dirname(args.output)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)

    with open(args.output, "w", encoding="utf-8") as f:
        f.write(html_content)

    print(f"[SUCCESS] Professional notification HTML email written to: {args.output}")

if __name__ == "__main__":
    main()
